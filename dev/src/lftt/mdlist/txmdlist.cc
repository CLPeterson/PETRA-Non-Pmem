//------------------------------------------------------------------------------
// 
//     
//
//------------------------------------------------------------------------------

#include <array>
#include <cmath>
#include <climits>
#include "txmdlist.h"


#define CLR_MARKD(_p)    ((NodeDesc *)(((uintptr_t)(_p)) & ~1))
#define IS_MARKED(_p)     (((uintptr_t)(_p)) & 1)
#define SET_MARK_DESC(_p)   ((NodeDesc *)(((uintptr_t)(_p)) | 1))

#define SET_ADPINV(_p)    ((Node *)(((uintptr_t)(_p)) | 1))
#define CLR_ADPINV(_p)    ((Node *)(((uintptr_t)(_p)) & ~1))
#define IS_ADPINV(_p)     (((uintptr_t)(_p)) & 1)

#define CLR_INVALID(_p)    ((Node *)(((uintptr_t)(_p)) & ~3))
#define IS_INVALID(_p)     (((uintptr_t)(_p)) & 3)


#define SET_DELINV(_p)    ((Node *)(((uintptr_t)(_p)) | 2))
#define CLR_DELINV(_p)    ((Node *)(((uintptr_t)(_p)) & ~2))
#define IS_DELINV(_p)     (((uintptr_t)(_p)) & 2)

__thread TxMdList::HelpStack TxMdList_helpStack;

template<>
inline void TxMdList::KeyToCoord<16>(uint32_t key, uint8_t coord[])
{
    static const uint32_t MASK[16] = {0x3u << 30, 0x3u << 28, 0x3u << 26, 0x3u << 24, 0x3u << 22, 0x3u << 20, 0x3u << 18, 0x3u << 16,
                                      0x3u << 14, 0x3u << 12, 0x3u << 10, 0x3u << 8, 0x3u << 6, 0x3u << 4, 0x3u << 2, 0x3u};

    for (uint32_t i = 0; i < 16 ; ++i) 
    {
        coord[i] = ((key & MASK[i]) >> (30 - (i << 1)));
    }
}

// TxMdList::TxMdList(Allocator<Node>* nodeAllocator, Allocator<Desc>* descAllocator, Allocator<NodeDesc>* nodeDescAllocator, Allocator<AdoptDesc>* _AdoptDescAllocator,uint32_t keyRange)
TxMdList::TxMdList(Allocator<Node>* nodeAllocator, Allocator<Desc>* descAllocator, Allocator<NodeDesc>* nodeDescAllocator, uint32_t keyRange)
    : m_head(new Node)
    , m_basis(3 + std::ceil(std::pow((float)keyRange, 1.0 / (float)DIMENSION)))
    , m_nodeAllocator(nodeAllocator)
    , m_descAllocator(descAllocator)
    , m_nodeDescAllocator(nodeDescAllocator)
    // , m_AdoptDescAllocator(_AdoptDescAllocator)
{
    lastTxId.store(0);
    m_AdoptDescAllocator = nullptr;

}

TxMdList::~TxMdList()
{
    printf("Total commit %u, abort (total/fake) %u/%u\n", g_count_commit, g_count_abort, g_count_fake_abort);
    // Print();

    ASSERT_CODE(
    printf("Total node count %u, Inserts %u/%u, Deletions %u/%u\n", g_count, g_count_ins, g_count_ins_attempt, g_count_del, g_count_del_attempt);
    std::string prefix;
    Traverse(m_head, NULL, 0, prefix);
    )

    FILE* pfile = fopen("output.txt", "a"); //CORRECTNESS ANNOTATIONS

    fprintf(pfile, "%u ", g_count_commit); //CORRECTNESS ANNOTATIONS

    fclose(pfile); //CORRECTNESS ANNOTATIONS
}

bool TxMdList::ExecuteOps(Desc* desc)
{
    TxMdList_helpStack.Init();

    HelpOps(desc, 0);

    bool ret = desc->status != ABORTED;

    ASSERT_CODE
    (
        if(ret)
        {
            for(uint32_t i = 0; i < desc->size; ++i)
            {
                if(desc->ops[i].type == INSERT)
                {
                    __sync_fetch_and_add(&g_count, 1);
                }
                else if(desc->ops[i].type == DELETE)
                {
                    __sync_fetch_and_sub(&g_count, 1);
                }
                else
                {
                    __sync_fetch_and_add(&g_count_fnd, 1);
                }
            }
        }
    );

    return ret;
}

PERSIST_CODE
(

inline bool TxMdList::needPersistenceHelp(Desc* desc)
{
    return desc->persistStatus != PERSISTED;
}

inline void TxMdList::persistDesc(Desc* desc)
{
    for(int i = 0; i < desc->size; i++) 
    {
        DTX::PERSIST_FLUSH_ONLY(&(desc->ops[i]), sizeof(Operator));
    }
    desc->txid = lastTxId++;
    DTX::PERSIST_FLUSH_ONLY(desc, sizeof(Desc));
    DTX::PERSIST_BARRIER_ONLY();
    desc->persistStatus = PERSISTED;
}

)

inline void TxMdList::HelpOps(Desc* desc, uint8_t opid)
{
    if(desc->status != ACTIVE)
    {
        bool needHelp = false;
        PERSIST_CODE
        (
            needHelp = needPersistenceHelp(desc);
        )

READ_ONLY_OPT_CODE
(
        if(desc->isReadOnly)
            needHelp = false;
)         
        if(!needHelp)
            return;
    }

    //Cyclic dependcy check
    if(TxMdList_helpStack.Contain(desc))
    {
        if(__sync_bool_compare_and_swap(&desc->status, ACTIVE, ABORTED))
        {
            PERSIST_CODE
            (
                if(__sync_bool_compare_and_swap(&desc->persistStatus, MAYBE, IN_PROGRESS)) 
                {
                    persistDesc(desc);
                }else if(desc->persistStatus == IN_PROGRESS)
                {
                    persistDesc(desc);
                }

            )            
            __sync_fetch_and_add(&g_count_abort, 1);
            __sync_fetch_and_add(&g_count_fake_abort, 1);
        }else {
            PERSIST_CODE
            (
                if(needPersistenceHelp(desc)) 
                {
                    if(__sync_bool_compare_and_swap(&desc->persistStatus, MAYBE, IN_PROGRESS)) 
                    {
                        persistDesc(desc);
                    }else if(desc->persistStatus == IN_PROGRESS)
                    {
                        persistDesc(desc);
                    }
                }
            )            
        }

        return;
    }

    ReturnCode ret = OK;
    std::vector<Node*> delNodes;
    std::vector<Node*> delPredNodes;
    std::vector<u_int32_t> delPredDims;
    std::vector<Node*> insNodes;
    std::vector<Node*> insPredNodes;
    std::vector<u_int32_t> insPredDims;

    TxMdList_helpStack.Push(desc);

    while(desc->status == ACTIVE && ret != FAIL && opid < desc->size)
    {

        const Operator& op = desc->ops[opid];

        // if(opid > 0) {
        //     printf("finished opid=%d and key %d\n\r", opid - 1, desc->ops[opid - 1]);
        // }
        // printf("started opid=%d and key %d\n\r", opid, desc->ops[opid]);

        if(op.type == INSERT)
        {
            Node* inserted;
            Node* pred;
            u_int32_t dim, pred_dim;
            ret = Insert(op.key, desc, opid, inserted, pred, dim, pred_dim);
                

            insNodes.push_back(inserted);
            insPredNodes.push_back(pred);
            insPredDims.push_back(pred_dim);

        }
        else if(op.type == DELETE)
        {
            Node* deleted;
            Node* pred;
            u_int32_t dim, pred_dim;
            ret = Delete(op.key, desc, opid, deleted, pred,  dim, pred_dim);            

            delNodes.push_back(deleted);
            delPredNodes.push_back(pred);
            delPredDims.push_back(pred_dim);

        }
        else
        {
            ret = Find(op.key, desc, opid);

        }
        
        opid++;
    }
    // printf("finished opid=%d and key %d\n\r", opid - 1, desc->ops[opid - 1]);

    TxMdList_helpStack.Pop();

    if(ret != FAIL)
    {
READ_ONLY_OPT_CODE        
(
    if(desc->isReadOnly) {
        if(__sync_bool_compare_and_swap(&desc->status, ACTIVE, COMMITTED))
        {
            __sync_fetch_and_add(&g_count_commit, 1);        
        }
    }else {

        if(__sync_bool_compare_and_swap(&desc->status, ACTIVE, COMMITTED))
        {
            PERSIST_CODE
            (
                if(__sync_bool_compare_and_swap(&desc->persistStatus, MAYBE, IN_PROGRESS))
                {
                    persistDesc(desc);
                }else if(desc->persistStatus == IN_PROGRESS)
                {
                    persistDesc(desc);
                }
            )  
            MarkForDeletion(delNodes, delPredNodes, delPredDims, desc);
            __sync_fetch_and_add(&g_count_commit, 1);
        }        
    }
 )

NO_READ_ONLY_OPT_CODE        
(       
        if(__sync_bool_compare_and_swap(&desc->status, ACTIVE, COMMITTED))
        {     
            PERSIST_CODE
            (
                if(__sync_bool_compare_and_swap(&desc->persistStatus, MAYBE, IN_PROGRESS))
                {
                    persistDesc(desc);
                }else if(desc->persistStatus == IN_PROGRESS)
                {
                    persistDesc(desc);
                }
            )  
            
            MarkForDeletion(delNodes, delPredNodes, delPredDims, desc);
            __sync_fetch_and_add(&g_count_commit, 1);
        }            
 )            

    }
    else
    {
READ_ONLY_OPT_CODE        
(
    if(desc->isReadOnly) {
        if(__sync_bool_compare_and_swap(&desc->status, ACTIVE, ABORTED))
        {
            __sync_fetch_and_add(&g_count_abort, 1);        
        }
    }else {
        if(__sync_bool_compare_and_swap(&desc->status, ACTIVE, ABORTED))
        {
            PERSIST_CODE
            (
                if(__sync_bool_compare_and_swap(&desc->persistStatus, MAYBE, IN_PROGRESS))
                {
                    persistDesc(desc);
                }else if(desc->persistStatus == IN_PROGRESS)
                {
                    persistDesc(desc);
                }

            )  
            MarkForDeletion(insNodes, insPredNodes, insPredDims, desc);
            __sync_fetch_and_add(&g_count_abort, 1);
        }            
    }       

)

NO_READ_ONLY_OPT_CODE        
(
        if(__sync_bool_compare_and_swap(&desc->status, ACTIVE, ABORTED))
        {
            PERSIST_CODE
            (
                if(__sync_bool_compare_and_swap(&desc->persistStatus, MAYBE, IN_PROGRESS))
                {
                    persistDesc(desc);
                }else if(desc->persistStatus == IN_PROGRESS)
                {
                    persistDesc(desc);
                }

            )  
            MarkForDeletion(insNodes, insPredNodes, insPredDims, desc);
            __sync_fetch_and_add(&g_count_abort, 1);
        } 
)     
    }

}


inline bool TxMdList::IsNodeExist(Node* node, uint32_t key)
{
    return node != NULL && node->m_key == key;
}

inline TxMdList::ReturnCode TxMdList::Insert(uint32_t key, Desc* desc, uint8_t opid, Node*& inserted, Node*& pred, uint32_t& dim, uint32_t& pred_dim)
{
    //Allocate new node
    //Decompose Key into mutli-dimension coordinates
    NodeDesc* nodeDesc = new(m_nodeDescAllocator->Alloc()) NodeDesc(desc, opid);
    Node* new_node = new(m_nodeAllocator->Alloc()) Node();
    new_node->m_key = key;
    new_node->nodeDesc = nodeDesc;
    KeyToCoord<DIMENSION>(key, new_node->m_coord);

    inserted = NULL;
    Node* curr = m_head;
    dim = 0;       //the dimension of curr node
    pred_dim = 0;  //the dimesnion of pred node    
    // int count = 0;
    while(true)
    {
        
        LocatePred(new_node->m_coord, pred, curr, dim, pred_dim);
        Node* pred_child = pred->m_child[pred_dim];
        // printf("I start%d -> %p <> %p\n\r", count++, pred_child, curr);  
        // If node exists
        if(dim == DIMENSION  && !IS_DELINV(pred_child) /* && IsNodeExist(curr, key)*/)
        {
            NodeDesc* oldCurrDesc = curr->nodeDesc;


            if(IS_MARKED(oldCurrDesc))
            {
                if(!IS_DELINV(pred->m_child[pred_dim]))
                {
                    (__sync_fetch_and_or(&pred->m_child[pred_dim], 0x2));
                }
                curr = m_head;
                continue;
            }


            FinishPendingTxn(oldCurrDesc, desc);

            if(IsSameOperation(oldCurrDesc, nodeDesc))
            {
                return SKIP;
            }
            //if(!IsKeyExist(oldCurrDesc))
			if(!IsKeyExist_mod(oldCurrDesc, desc))
            {
                NodeDesc* currDesc = curr->nodeDesc;

                if(desc->status != ACTIVE)
                {
                    return FAIL;
                }

                currDesc = __sync_val_compare_and_swap(&curr->nodeDesc, oldCurrDesc, nodeDesc);

                if(currDesc == oldCurrDesc)
                {
                    ASSERT_CODE
                        (
                            __sync_fetch_and_add(&g_count_ins, 1);
                        );

                    inserted = curr;
                    return OK; 
                }
                // printf("I%d -> %p <> %p\n\r", count++, currDesc, oldCurrDesc);                                

            }
            else 
            {
                return FAIL;
            }            

        }
        // If node does not exist
        else
        {
            if(desc->status != ACTIVE)
            {
                return FAIL;
            }

            //There are three possible inserting positions:
            //1. on link edge from pred to curr node
            //   [repalce pointer in pred with new_node and make curr the child of new_node] 
            //
            //2. before curr node
            //   [replace pointer in pred with new_node and copy all children up to dim to new_node] 
            //
            //3. after pred node
            //   [curr node must be NULL, no need to pend task]
            //
            //4. override the current node
            //   [insert new_node and disconnet curr node]

            //Atomically update pred node's child pointer
            //if cas fails, it means another node has succesfully inserted inbetween pred and curr
            Node* expected = curr;
            if(IS_DELINV(pred_child))
            {
                // printf("I%d -> changed expected\n\r", count++);
                expected = SET_DELINV(curr); 
                //if child adoption is need, we take the chance to do physical deletion
                //Otherwise, we CANNOT force full scale adoption
                if(dim == DIMENSION - 1)
                {
                    dim = DIMENSION;
                }
            }
            if(pred_child == expected)
            {
                // printf("I%d -> pred_child == expected\n\r", count++); 
                AdoptDesc* adoptDesc = FillNewNode(new_node, pred, expected, dim, pred_dim);
                pred_child = __sync_val_compare_and_swap(&pred->m_child[pred_dim], expected, new_node);
                if(pred_child == expected)
                {
                    //The insertion succeeds, complete remaining pending task for this node if any
                    if(adoptDesc) 
                    {
                        //Case 2 inserting new_node at the same dimesion as pred node
                        //and push the curr node down one dimension
                        //We need to help curr node finish the pending task 
                        //because the children adoption in new_node might need the adoption from curr node
                        AdoptDesc* pending = curr ? curr->m_pending : NULL;
                        if(pending)
                        {
                            FinishInserting(curr, pending);
                        }

                        FinishInserting(new_node, adoptDesc);
                    }
                    ASSERT_CODE(
                            if(dim != DIMENSION){
                            __sync_fetch_and_add(&g_count, 1);
                            __sync_fetch_and_add(&g_count_ins, 1);}
                            //printf("[%u] Inserting node %p key %u as %dth child of node %p key %u\n", g_count_ins_attempt, new_node, new_node->m_key, pred_dim, pred, pred ? pred->m_key : 0);
                            ) 
                    inserted = new_node;
                    return OK;                                       

                } 
                // printf("I%d -> pred_child != expected\n\r", count++);               
            }
            //If the code reaches here it means the CAS failed
            //Three reasons why CAS may fail:
            //1. the child slot has been marked as invalid by parents
            //2. another thread inserted a child into the slot
            //3. another thread deleted the child
            if(IS_ADPINV(pred_child))
            {
                pred = NULL;
                curr = m_head;
                dim = 0;
                pred_dim = 0;
                // printf("I%d -> r1 %p <> %p <> %p \n\r", count++, pred_child, expected, curr);
            }
            else if(CLR_INVALID(pred_child) != curr)
            {
                curr = pred;
                dim = pred_dim;
                // printf("I%d -> r2 \n\r", count++);
            }
            else
            {
                //Do nothing, no need to find new inserting poistion
                //retry insertion at the same location
                // printf("I%d -> r3 \n\r", count++);
            }

            //If pending taks is allocate free it now
            //it might not be needed in the next try
            //No need to restore other fields as they will be initilized in the next iteration 
            if(new_node->m_pending)
            {
                delete new_node->m_pending;
                new_node->m_pending = NULL;
            }                                                                        
        }        
    }

}

inline TxMdList::AdoptDesc* TxMdList::FillNewNode(Node* new_node, Node*& pred, Node*& curr, uint32_t& dim, uint32_t& pred_dim)
{
    AdoptDesc* desc = NULL;
    if(pred_dim != dim)
    {
        //descriptor to instruct other insertion task to help migrate the children
        // desc = new(m_AdoptDescAllocator->Alloc()) AdoptDesc();
        desc = new AdoptDesc();

        desc->curr = CLR_DELINV(curr);
        desc->pred_dim = pred_dim;
        desc->dim = dim;
    }

    //Fill values for new_node, m_child is set to 1 for all children before pred_dim
    //pred_dim is the dimension where new_node is inserted, all dimension before that are invalid for new_node
    for(uint32_t i = 0; i < pred_dim; ++i)
    {
        new_node->m_child[i] = (Node*)0x1;
    }
    //be careful with the length of memset, should be DIMENSION - pred_dim NOT (DIMENSION - 1 - pred_dim)
    memset(new_node->m_child + pred_dim, 0, sizeof(Node*) * (DIMENSION - pred_dim));
    if(dim < DIMENSION)
    {
        //If curr is marked for deletion or overriden, we donnot link it. 
        //Instead, we adopt ALL of its children
        new_node->m_child[dim] = curr;
    }
    new_node->m_pending = desc;

    return desc;
}

inline void TxMdList::FinishInserting(Node* n, AdoptDesc* desc)
{
    uint32_t pred_dim = desc->pred_dim;    
    uint32_t dim = desc->dim;    
    Node* curr = desc->curr;

    for (uint32_t i = pred_dim; i < dim; ++i) 
    {
        Node* child = curr->m_child[i];

        //Children slot of curr_node need to be marked as invalid 
        //before we copy them to new_node
        //while(!IS_ADPINV(child) && !__sync_bool_compare_and_swap(&curr->m_child[i], child, SET_ADPINV(child)))
        //{
            //child = curr->m_child[i];
        //}
        child = __sync_fetch_and_or(&curr->m_child[i], 0x1);
        child = CLR_ADPINV(curr->m_child[i]);
        if(child)
        {
            //Adopt children from curr_node's
            if(n->m_child[i] == NULL)
            {
                __sync_bool_compare_and_swap(&n->m_child[i], NULL, child);
            }
        }
    }

    //Clear the pending task
    if(n->m_pending == desc && __sync_bool_compare_and_swap(&n->m_pending, desc, NULL))
    {
        //TODO:recycle desc
    }
}

inline TxMdList::ReturnCode TxMdList::Delete(uint32_t key, Desc* desc, uint8_t opid, Node*& deleted, Node*& pred, uint32_t& dim, uint32_t& pred_dim)
{
    deleted = NULL;
    NodeDesc* nodeDesc = new(m_nodeDescAllocator->Alloc()) NodeDesc(desc, opid);
    uint8_t coord[DIMENSION];
    KeyToCoord<DIMENSION>(key, coord);
    Node* curr = m_head;    //curr node
    dim = 0;       //the dimension of curr node
    pred_dim = 0;  //the dimesnion of pred node

    // int count = 0;
    while(true)
    {
        // printf("D%d\n\r", count++);
        LocatePred(coord, pred, curr, dim, pred_dim);
        if(dim == DIMENSION)
        {
            NodeDesc* oldCurrDesc = curr->nodeDesc;
            if(IS_MARKED(oldCurrDesc))
            {
                return FAIL;
                //Help removed deleted nodes
                //if(!IS_MARKED(curr->next))
                //{
                    //__sync_fetch_and_or(&curr->next, 0x1);
                //}
                //curr = m_head;
                //continue;
            }
            
            FinishPendingTxn(oldCurrDesc, desc); 

            if(IsSameOperation(oldCurrDesc, nodeDesc))
            {
                return SKIP;
            }
            //if(IsKeyExist(oldCurrDesc))
			if(IsKeyExist_mod(oldCurrDesc, desc))
            {
                NodeDesc* currDesc = curr->nodeDesc;

                if(desc->status != ACTIVE)
                {
                    return FAIL;
                }
                //Update desc 
                currDesc = __sync_val_compare_and_swap(&curr->nodeDesc, oldCurrDesc, nodeDesc);
                if(currDesc == oldCurrDesc)
                {
                    ASSERT_CODE
                        (
                            __sync_fetch_and_add(&g_count_del, 1);
                        );
                    deleted = curr;
                    return OK; 
                }
            }
            else
            {
                return FAIL;
            }                                   
        }
        else
        {
            // printf("D%d not exist\n\r", count++);
            return FAIL;
        }
      
    }

}

void TxMdList::ResetMetrics()
{
    g_count_commit = 0;
    g_count_abort = 0;
    g_count_fake_abort = 0;
}


void TxMdList::MarkForDeletion(const std::vector<Node*>& nodes, const std::vector<Node*>& preds, const std::vector<uint32_t>& pred_dims, Desc* desc)
{
    // Mark nodes for logical deletion
    for(uint32_t i = 0; i < nodes.size(); ++i)
    {
        Node* n = nodes[i];
        if(n != NULL)
        {
            NodeDesc* nodeDesc = CLR_MARKD(n->nodeDesc);
            if(nodeDesc->desc == desc)
            {
                if(__sync_bool_compare_and_swap(&n->nodeDesc, nodeDesc, SET_MARK_DESC(nodeDesc)))
                {
                    Node* pred = preds[i];
                    uint32_t pred_dim = pred_dims[i];
                    __sync_val_compare_and_swap(&pred->m_child[pred_dim], n, SET_DELINV(n));
                }

            }
        }

    }
}

inline TxMdList::ReturnCode TxMdList::Find(uint32_t key, Desc* desc, uint8_t opid)
{

    //TODO: may be use specilized locatedPred to speedup
    uint8_t coord[DIMENSION];
    KeyToCoord<DIMENSION>(key, coord);
    Node* pred = NULL;      //pred node
    Node* curr = m_head;    //curr node
    uint32_t dim = 0;       //the dimension of curr node
    uint32_t pred_dim = 0;  //the dimesnion of pred node

    NodeDesc* nodeDesc = new(m_nodeDescAllocator->Alloc()) NodeDesc(desc, opid);


    // int count = 0;
     while(true)
    {
        // printf("f%d\n\r", count++);
        LocatePred(coord, pred, curr, dim, pred_dim);
        if(dim == DIMENSION)
        {
            // NodeDesc* oldCurrDesc = CLR_MARKD(curr->nodeDesc);
            NodeDesc* oldCurrDesc = curr->nodeDesc;
            // if(IS_MARKED(oldCurrDesc))
            // {
            //     if(!IS_DELINV(pred->m_child[pred_dim]))
            //     {
            //         (__sync_fetch_and_or(&pred->m_child[pred_dim], 0x2));
            //     }
            //     curr = m_head;
            //     continue;
            // }
            if(IS_MARKED(oldCurrDesc))
            {
                return FAIL;
                //Help removed deleted nodes
                //if(!IS_MARKED(curr->next))
                //{
                    //__sync_fetch_and_or(&curr->next, 0x1);
                //}
                //curr = m_head;
                //continue;
            }                        
            FinishPendingTxn(oldCurrDesc, desc); 

            if(IsSameOperation(oldCurrDesc, nodeDesc))
            {
                return SKIP;
            }
            //if(IsKeyExist(oldCurrDesc))
			if(IsKeyExist_mod(oldCurrDesc, desc))
            {
                NodeDesc* currDesc = curr->nodeDesc;

                if(desc->status != ACTIVE)
                {
                    return FAIL;
                }
// READ_ONLY_OPT_CODE
// (
//         if(desc->isReadOnly)
//             return OK;
// )                 
                //Update desc 
                currDesc = __sync_val_compare_and_swap(&curr->nodeDesc, oldCurrDesc, nodeDesc);
                if(currDesc == oldCurrDesc)
                {
                    return OK; 
                }                
            }
            else
            {
                return FAIL;
            }               
        }
        else
        {
            return FAIL;
        }           
    }
}

inline bool TxMdList::IsSameOperation(NodeDesc* nodeDesc1, NodeDesc* nodeDesc2)
{
    return CLR_MARKD(nodeDesc1)->desc == CLR_MARKD(nodeDesc2)->desc && CLR_MARKD(nodeDesc1)->opid == CLR_MARKD(nodeDesc2)->opid;
}

inline void TxMdList::FinishPendingTxn(NodeDesc* nodeDesc, Desc* desc)
{
    // nodeDesc = CLR_MARKD(nodeDesc);
    // The node accessed by the operations in same transaction is always active 
    if(nodeDesc->desc == desc)
    {
        return;
    }

    HelpOps(nodeDesc->desc, nodeDesc->opid + 1);
}

bool TxMdList::IsNodeActive(NodeDesc* nodeDesc)
{
    bool ret = (CLR_MARKD(nodeDesc)->desc->status == COMMITTED);

    PERSIST_CODE
    (
        ret = ret && (CLR_MARKD(nodeDesc)->desc->persistStatus == PERSISTED);
    )

    return ret;
}

inline bool TxMdList::IsKeyExist(NodeDesc* nodeDesc)
{
    bool isNodeActive = IsNodeActive(nodeDesc);
    uint8_t opType = CLR_MARKD(nodeDesc)->desc->ops[nodeDesc->opid].type;

    return  (opType == FIND) || (isNodeActive && opType == INSERT) || (!isNodeActive && opType == DELETE);
}

bool TxMdList::IsNodeActive_mod(NodeDesc* nodeDesc, Desc* desc_this)
{
    bool ret = (CLR_MARKD(nodeDesc)->desc->status == COMMITTED);

    PERSIST_CODE
    (
        ret = ret && (CLR_MARKD(nodeDesc)->desc->persistStatus == PERSISTED);
    )

	//BUG FIX
	ret = ret || (nodeDesc->desc == desc_this);

    return ret;
}

inline bool TxMdList::IsKeyExist_mod(NodeDesc* nodeDesc, Desc* desc_this)
{
    bool isNodeActive = IsNodeActive_mod(nodeDesc, desc_this);
    uint8_t opType = CLR_MARKD(nodeDesc)->desc->ops[nodeDesc->opid].type;

    return  (opType == FIND) || (isNodeActive && opType == INSERT) || (!isNodeActive && opType == DELETE);
}


inline void TxMdList::LocatePred(uint8_t coord[], Node*& pred, Node*& curr, uint32_t& dim, uint32_t& pred_dim)
{
    //Locate the proper position to insert
    //traverse list from low dim to high dim
    while(dim < DIMENSION)
    {
        //Loacate predecessor and successor
        while(curr && coord[dim] > curr->m_coord[dim])
        {
            pred_dim = dim;
            pred = curr;

            AdoptDesc* pending = curr->m_pending;
            if(pending && dim >= pending->pred_dim && dim <= pending->dim)
            {
                // printf("FinishInserting on %p\n\r", curr);
                FinishInserting(curr, pending);

            }
            curr = CLR_INVALID(curr->m_child[dim]);
        }

        //no successor has greater coord at this dimension
        //the position after pred is the insertion position
        if(curr == NULL || coord[dim] < curr->m_coord[dim]) 
        {
            //done searching
            break;
        }
        //continue to search in the next dimension 
        //if coord[dim] of new_node overlaps with that of curr node
        else
        {
            //dim only increases if two coords are exactly the same
            ++dim;
        }
    }
}


inline void TxMdList::Print()
{
    std::string prefix;
    Traverse(m_head, NULL, 0, prefix);
}

inline void TxMdList::Traverse(Node* n, Node* parent, int dim, std::string& prefix)
{
    if(!IS_DELINV(n))
    {
        printf("%s", prefix.c_str());
        printf("Node [%p] Key [%u] DIM [%d] of Parent[%p]\n", n, n->m_key, dim, parent);
    }
    n = CLR_DELINV(n);   
    //traverse from last dimension up to current dim
    //The valid children include child nodes up to dim
    //e.g. a node on dimension 3 has only valid children on dimensions 3~8
    for (int i = DIMENSION - 1; i >= dim; --i) 
    {
        Node* child = n->m_child[i];
        ASSERT(!IS_ADPINV(child), "corrupt mdlist, invalid child in a valid dimension");

        if(child != NULL)
        {
            prefix.push_back('|');
            prefix.insert(prefix.size(), i, ' ');

            Traverse(child, n, i, prefix);

            prefix.erase(prefix.size() - i - 1, i + 1);
        }
    }    
}
