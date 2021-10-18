//------------------------------------------------------------------------------
// 
//     
//
//------------------------------------------------------------------------------


#include <cstdlib>
#include <cstdio>
#include <new>
#include <iostream>
#include <set>
#include <map>
#include <climits>
#include "translist.h"

#define SET_MARK(_p)    ((Node *)(((uintptr_t)(_p)) | 1))
#define CLR_MARK(_p)    ((Node *)(((uintptr_t)(_p)) & ~1))
#define CLR_MARKD(_p)    ((NodeDesc *)(((uintptr_t)(_p)) & ~1))
#define IS_MARKED(_p)     (((uintptr_t)(_p)) & 1)

__thread TransList::HelpStack helpStack;

TransList::TransList(Allocator<Node>* nodeAllocator, Allocator<Desc>* descAllocator, Allocator<NodeDesc>* nodeDescAllocator, bool newList)
    : m_nodeAllocator(nodeAllocator)
    , m_descAllocator(descAllocator)
    , m_nodeDescAllocator(nodeDescAllocator)
{
    lastTxId.store(0);
    if(newList) {
        m_tail = new (m_nodeAllocator->Alloc()) Node(0xffffffff, NULL, NULL);
        m_head = new (m_nodeAllocator->Alloc()) Node(0, m_tail, NULL);
    }else {
ALLOCATOR_PERSISTABLE_ONLY_CODE
(        
        m_tail = m_nodeAllocator->getFirst();
        m_head = m_nodeAllocator->getNext(m_tail);
)
    }    
}

TransList::~TransList()
{
    printf("Total commit %u, abort (total/fake) %u/%u\n", g_count_commit, g_count_abort, g_count_fake_abort);
    // Print();

    ASSERT_CODE
    (
        printf("Total node count %u, Inserts (total/new) %u/%u, Deletes (total/new) %u/%u, Finds %u\n", g_count, g_count_ins, g_count_ins_new, g_count_del , g_count_del_new, g_count_fnd);
    );

    //Node* curr = m_head;
    //while(curr != NULL)
    //{
        //free(curr);
        //curr = curr->next;
    //}
   
    FILE* pfile = fopen("output.txt", "a"); //CORRECTNESS ANNOTATIONS

    fprintf(pfile, "%u ", g_count_commit); //CORRECTNESS ANNOTATIONS

    fclose(pfile); //CORRECTNESS ANNOTATIONS
}


TransList::Desc* TransList::AllocateDesc(uint8_t size)
{
    Desc* desc = m_descAllocator->Alloc();
    desc->size = size;
    desc->status = ACTIVE;
    desc->persistStatus = MAYBE;
    
    return desc;
}

bool TransList::ExecuteOps(Desc* desc)
{
    helpStack.Init();

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


void TransList::ResetMetrics()
{
    g_count_commit = 0;
    g_count_abort = 0;
    g_count_fake_abort = 0;
}

inline void TransList::MarkForDeletion(const std::vector<Node*>& nodes, const std::vector<Node*>& preds, Desc* desc)
{
    // Mark nodes for logical deletion
    for(uint32_t i = 0; i < nodes.size(); ++i)
    {
        Node* n = nodes[i];
        if(n != NULL)
        {
            NodeDesc* nodeDesc = n->nodeDesc;

            if(nodeDesc->desc == desc)
            {
                if(__sync_bool_compare_and_swap(&n->nodeDesc, nodeDesc, SET_MARK(nodeDesc)))
                {
                    Node* pred = preds[i];
                    Node* succ = CLR_MARK(__sync_fetch_and_or(&n->next, 0x1));
                    __sync_bool_compare_and_swap(&pred->next, n, succ);
                }
            }
        }
    }
}


PERSIST_CODE
(

inline bool TransList::needPersistenceHelp(Desc* desc)
{
    return desc->persistStatus != PERSISTED;
}


inline void TransList::persistDesc(Desc* desc)
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


inline void TransList::HelpOps(Desc* desc, uint8_t opid)
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
    if(helpStack.Contain(desc))
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
    std::vector<Node*> insNodes;
    std::vector<Node*> insPredNodes;

    helpStack.Push(desc);

    while(desc->status == ACTIVE && ret != FAIL && opid < desc->size)
    {

        const Operator& op = desc->ops[opid];

        if(op.type == INSERT)
        {
            Node* inserted;
            Node* pred;
            ret = Insert(op.key, desc, opid, inserted, pred);

            insNodes.push_back(inserted);
            insPredNodes.push_back(pred);

        }
        else if(op.type == DELETE)
        {
            Node* deleted;
            Node* pred;
            ret = Delete(op.key, desc, opid, deleted, pred);            

            delNodes.push_back(deleted);
            delPredNodes.push_back(pred);

        }
        else
        {
            ret = Find(op.key, desc, opid);
        }
		//if(ret != FAIL) insert_method();
        
        opid++;
    }

    helpStack.Pop();

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
                MarkForDeletion(delNodes, delPredNodes, desc);
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
            MarkForDeletion(delNodes, delPredNodes, desc);
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
            MarkForDeletion(insNodes, insPredNodes, desc);
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
                MarkForDeletion(insNodes, insPredNodes, desc);
            __sync_fetch_and_add(&g_count_abort, 1);
        } 
) 
    
    }

}

inline TransList::ReturnCode TransList::Insert(uint32_t key, Desc* desc, uint8_t opid, Node*& inserted, Node*& pred)
{
    inserted = NULL;
    NodeDesc* nodeDesc = new(m_nodeDescAllocator->Alloc()) NodeDesc(desc, opid);
    Node* new_node = NULL;
    Node* curr = m_head;

    while(true)
    {
        LocatePred(pred, curr, key);

        if(!IsNodeExist(curr, key))
        {
            //Node* pred_next = pred->next;

            if(desc->status != ACTIVE)
            {
                return FAIL;
            }

            //if(pred_next == curr)
            //{
                if(new_node == NULL)
                {
                    new_node = new(m_nodeAllocator->Alloc()) Node(key, NULL, nodeDesc);
                }
                new_node->next = curr;

                Node* pred_next = __sync_val_compare_and_swap(&pred->next, curr, new_node);

                if(pred_next == curr)
                {
                    ASSERT_CODE
                        (
                         __sync_fetch_and_add(&g_count_ins, 1);
                         __sync_fetch_and_add(&g_count_ins_new, 1);
                        );

                    inserted = new_node;
                    return OK;
                }
            //}

            // Restart
            curr = IS_MARKED(pred_next) ? m_head : pred;
        }
        else 
        {
            NodeDesc* oldCurrDesc = curr->nodeDesc;

            if(IS_MARKED(oldCurrDesc))
            {
                if(!IS_MARKED(curr->next))
                {
                    (__sync_fetch_and_or(&curr->next, 0x1));
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

                //if(currDesc == oldCurrDesc)
                {
                    //Update desc 
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
                }
            }
            else
            {
                return FAIL;
            }
        }
    }
}

inline TransList::ReturnCode TransList::Delete(uint32_t key, Desc* desc, uint8_t opid, Node*& deleted, Node*& pred)
{
    deleted = NULL;
    NodeDesc* nodeDesc = new(m_nodeDescAllocator->Alloc()) NodeDesc(desc, opid);
    Node* curr = m_head;

    while(true)
    {
        LocatePred(pred, curr, key);

        if(IsNodeExist(curr, key))
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

                //if(currDesc == oldCurrDesc)
                {
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


inline bool TransList::IsSameOperation(NodeDesc* nodeDesc1, NodeDesc* nodeDesc2)
{
    return nodeDesc1->desc == nodeDesc2->desc && nodeDesc1->opid == nodeDesc2->opid;
}


inline TransList::ReturnCode TransList::Find(uint32_t key, Desc* desc, uint8_t opid)
{
    NodeDesc* nodeDesc = NULL;
    Node* pred;
    Node* curr = m_head;

    while(true)
    {
        LocatePred(pred, curr, key);

        if(IsNodeExist(curr, key))
        {
            
            NodeDesc* oldCurrDesc = curr->nodeDesc;

            if(IS_MARKED(oldCurrDesc))
            {
                if(!IS_MARKED(curr->next))
                {
                    (__sync_fetch_and_or(&curr->next, 0x1));
                }
                curr = m_head;
                continue;
            }

            FinishPendingTxn(oldCurrDesc, desc);

            if(nodeDesc == NULL) nodeDesc = new(m_nodeDescAllocator->Alloc()) NodeDesc(desc, opid);

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

                //if(currDesc == oldCurrDesc)
                {
                    //Update desc 
                    currDesc = __sync_val_compare_and_swap(&curr->nodeDesc, oldCurrDesc, nodeDesc);

                    if(currDesc == oldCurrDesc)
                    {
                        return OK; 
                    }
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

inline bool TransList::IsNodeExist(Node* node, uint32_t key)
{
    return node != NULL && node->key == key;
}

inline void TransList::FinishPendingTxn(NodeDesc* nodeDesc, Desc* desc)
{
    // The node accessed by the operations in same transaction is always active 
    if(nodeDesc->desc == desc)
    {
        return;
    }

    HelpOps(nodeDesc->desc, nodeDesc->opid + 1);
}

inline bool TransList::IsNodeActive(NodeDesc* nodeDesc)
{
    bool ret = (nodeDesc->desc->status == COMMITTED);

    PERSIST_CODE
    (
        ret = ret && (nodeDesc->desc->persistStatus == PERSISTED);
    )

    return ret;
}

inline bool TransList::IsKeyExist(NodeDesc* nodeDesc)
{
    bool isNodeActive = IsNodeActive(nodeDesc);
    uint8_t opType = nodeDesc->desc->ops[nodeDesc->opid].type;

    return  (opType == FIND) || (isNodeActive && opType == INSERT) || (!isNodeActive && opType == DELETE);
}

inline bool TransList::IsNodeActive_mod(NodeDesc* nodeDesc, Desc* desc_this)
{
    bool ret = (nodeDesc->desc->status == COMMITTED);

    PERSIST_CODE
    (
        ret = ret && (nodeDesc->desc->persistStatus == PERSISTED);
    )

	//BUG FIX
	ret = ret || (nodeDesc->desc == desc_this);

    return ret;
}

inline bool TransList::IsKeyExist_mod(NodeDesc* nodeDesc, Desc* desc_this)
{
    bool isNodeActive = IsNodeActive_mod(nodeDesc, desc_this);
    uint8_t opType = nodeDesc->desc->ops[nodeDesc->opid].type;

    return  (opType == FIND) || (isNodeActive && opType == INSERT) || (!isNodeActive && opType == DELETE);
}

inline void TransList::LocatePred(Node*& pred, Node*& curr, uint32_t key)
{
    Node* pred_next;

    while(curr->key < key)
    {
        pred = curr;
        pred_next = CLR_MARK(pred->next);
        curr = pred_next;

        while(IS_MARKED(curr->next))
        {
            curr = CLR_MARK(curr->next);
        }

        if(curr != pred_next)
        {
            //Failed to remove deleted nodes, start over from pred
            if(!__sync_bool_compare_and_swap(&pred->next, pred_next, curr))
            {
                curr = m_head;
            }

            //__sync_bool_compare_and_swap(&pred->next, pred_next, curr);
        }
    }

    ASSERT(pred, "pred must be valid");
}

void TransList::CheckConsistency(std::set<int> existingKeySet)
{
    Node* curr = m_head->next;
    while(curr != m_tail) {
        std::set<int>::iterator it = existingKeySet.find(curr->key);
        if(it == existingKeySet.end()) {
            if(IsKeyExist(CLR_MARKD(curr->nodeDesc))) {
                std::cout << "Error: something is wrong with the key: " <<  curr->key <<  " it is not supposed to be in the list "  << std::endl;        
            } else {
                std::cout << curr->key << " is in the list, but is marked for deletion " << std::endl;        
            }
        } else {
            std::cout <<  curr->key << " is in the list" << std::endl;
            existingKeySet.erase(it);
        }
        curr = CLR_MARK(curr->next);
    }
    if(existingKeySet.size() > 0) {
        std::cout << "The following elements were supposed to be in the list, but they were not found:"<< std::endl;
        for(std::set<int>::iterator it = existingKeySet.begin(); it != existingKeySet.end(); ++it) {
            std::cout << "\t" << *it<< std::endl;
        }
    }
}

void TransList::fixPointers()
{
    Node* curr = m_head;
    curr->next = m_nodeAllocator->getNewPointer(m_head->next);
    curr = curr->next;
    int counter = 0;
    while(curr != nullptr && curr != m_tail && curr->next != nullptr && curr->next != m_tail) {
        //std::cout << counter++ << std::endl;
        Node* oldNext = curr->next;
        curr->next = m_nodeAllocator->getNewPointer(curr->next);
        //printf("%d: %p -> %p\n", curr->key, oldNext, curr->next);
        curr->nodeDesc = m_nodeDescAllocator->getNewPointer(curr->nodeDesc);
        curr->nodeDesc->desc = m_descAllocator->getNewPointer(curr->nodeDesc->desc);
        curr = curr->next;
        // printf("curr->next->key=%p, %d\n", &(curr->next->key), curr->next->key);
    }
}

void TransList::validate_and_recovery(int thread_count, Allocator<TransList::Desc>* l_descAllocator)
{
    //std::cout << "starting the validation " << std::endl;
    std::map<int,TransList::Desc*> descMap;
    // std::cout << "sizeof(Desc): " << sizeof(Desc) << std::endl;
    // for(int i = 0; i < thread_count; i++) {
    //     Desc *curr = l_descAllocator->getFirstForThread(i);
    //     std::cout << "thread(" << i << "): " << curr << std::endl; 
    //     printf("%p, %p, %d\n", curr, &(curr->size), curr->size);
    //     std::cout << &(curr->persistStatus) << std::endl;
    //     std::cout << &(curr->size) << std::endl;
    //     std::cout << &(curr->txid) << std::endl;
    //     std::cout << curr->SizeOf(1) << std::endl;
    //     std::cout << curr->txid << std::endl;
    // }

    //for(int i = 1; i < thread_count; i++) {
	for(int i = 0; i < thread_count + 1; i++) { //CORRECTNESS ANNOTATIONS
        //std::cout << "thread: " << i << std::endl;
        Desc *curr = l_descAllocator->getFirstForThread(i);
        Desc *next = l_descAllocator->getNextForThread(curr, i);
        int counter = 0;
        while(curr != nullptr) {
            // std::cout << counter++ << std::endl;
            if(curr->status == COMMITTED) {
                // uint32_t test = (uint32_t)curr->size;
                // std::cout << test << std::endl;
                for(uint32_t i = 0; i < curr->size; i++) {
                    std::map<int,TransList::Desc *>::iterator it = descMap.find(curr->ops[i].key);
                    if(it == descMap.end()) {
                        descMap[curr->ops[i].key] = curr;
                    } else {
                        TransList::Desc * found = it->second;
                         if(curr->txid > found->txid) {
                             descMap[curr->ops[i].key] = curr;
                         }
                    }
                }                
            }
            curr = next;
            next = l_descAllocator->getNextForThread(next, i); 
        }       
    }
    //std::cout << "before fixing pointers " << std::endl;
    fixPointers();
    //printf("m_head=%p, m_tail=%p, m_head->next=%p\n\r", m_head, m_tail, m_head->next);
    // Node* curr = m_nodeAllocator->getNewPointer(m_head->next);
    // printf("curr-key=%p\n", &(curr->key));
    Node* curr = m_head->next;
    int counter = 0;
    while(curr != m_tail) {
        //std::cout << "key:" << curr->key << std::endl;

        std::map<int,TransList::Desc *>::iterator it = descMap.find(curr->key);
        if(it == descMap.end()) {
            //std::cout << "if" << std::endl;
            if(IsKeyExist(CLR_MARKD(curr->nodeDesc))) {
                //std::cout << "Error: something is wrong with the key: " <<  curr->key <<  " it is not supposed to be in the list "  << std::endl;        
            } else {
                //std::cout << curr->key << " is in the list, but is marked for deletion " << std::endl;        
            }
        } else {
            //std::cout << "else" << std::endl;
            // std::cout <<  curr->key << " is in the list" << std::endl;
            if(it->second == curr->nodeDesc->desc)
                counter++;
            else
                ;//std::cout << "Error: something is wrong with the key: " <<  curr->key <<  " points to wrong desc "  << std::endl;        
            // existingKeySet.erase(it);
        }
        curr = CLR_MARK(curr->next);
    }
    //std::cout << counter  << " elements are correctly in the set " << std::endl;




}



inline void TransList::Print()
{
    Node* curr = m_head->next;

    while(curr != m_tail)
    {
        printf("Node [%p] Key [%u] Status [%s] txid [%u]\n", curr, curr->key, IsKeyExist(CLR_MARKD(curr->nodeDesc))? "Exist":"Inexist", CLR_MARKD(curr->nodeDesc)->desc->txid);
        curr = CLR_MARK(curr->next);
    }
}
