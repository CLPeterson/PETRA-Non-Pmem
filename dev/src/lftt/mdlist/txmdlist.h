#ifndef TRANSMDLIST_H
#define TRANSMDLIST_H

#include <cstdint>
#include <vector>
#include "../../common/assert.h"
#include "../../common/allocator.h"

#include "../../durabletxn/dtx.h"

class TxMdList
{
public:

    static const uint32_t DIMENSION = 16;
    struct Node;

    enum OpStatus
    {
        ACTIVE = 0,
        COMMITTED,
        ABORTED,
    };

    enum PersistStatus
    {
        MAYBE = 0,
        IN_PROGRESS,
        PERSISTED,
    };    

    enum ReturnCode
    {
        OK = 0,
        SKIP,
        FAIL
    };

    enum OpType
    {
        FIND = 0,
        INSERT,
        DELETE
    };

    struct Operator
    {
        uint8_t type;
        uint32_t key;
    };

    struct Desc
    {
        static size_t SizeOf(uint8_t size)
        {
            // return sizeof(uint8_t) + sizeof(uint8_t) + sizeof(Operator) * size;
            return sizeof(Desc) + sizeof(Operator) * size;
        }

        // Status of the transaction: values in [0, size] means live txn, values -1 means aborted, value -2 means committed.
        volatile uint8_t status;
        volatile uint8_t persistStatus;
        uint8_t size;
        bool isReadOnly;
        uint32_t txid;
        Operator ops[];
    };
    
    struct NodeDesc
    {
        NodeDesc(Desc* _desc, uint8_t _opid)
            : desc(_desc), opid(_opid){}

        Desc* desc;
        uint8_t opid;
    };

 //Any insertion as a child of node in the rage [pred_dim, dim] needs to help finish the task
    struct AdoptDesc
    {
        Node* curr;
        uint8_t pred_dim;              //dimension of pred node
        uint8_t dim;                   //dimension of this node
    };

    struct Node
    {
        // Node(): m_key(0), m_child(NULL), nodeDesc(NULL), m_coord(NULL), m_pending(NULL){}
        // Node(uint32_t _key, NodeDesc* _nodeDesc)
        //     : m_key(_key), m_child(NULL), nodeDesc(_nodeDesc), m_coord(NULL), m_pending(NULL){}

        uint32_t m_key;             //key
        AdoptDesc* m_pending;            //pending operation to adopt children 
        NodeDesc* nodeDesc;
        Node* m_child[DIMENSION];
        uint8_t m_coord[DIMENSION];

    };    

   
    struct HelpStack
    {
        void Init()
        {
            index = 0;
        }

        void Push(Desc* desc)
        {
            ASSERT(index < 255, "index out of range");

            helps[index++] = desc;
        }

        void Pop()
        {
            ASSERT(index > 0, "nothing to pop");

            index--;
        }

        bool Contain(Desc* desc)
        {
            for(uint8_t i = 0; i < index; i++)
            {
                if(helps[i] == desc)
                {
                    return true;
                }
            }

            return false;
        }

        Desc* helps[256];
        uint8_t index;
    };

    // TxMdList(Allocator<Node>* nodeAllocator, Allocator<Desc>* descAllocator, Allocator<NodeDesc>* nodeDescAllocator, Allocator<AdoptDesc>* _AdoptDescAllocator, uint32_t keyRange);
        TxMdList(Allocator<Node>* nodeAllocator, Allocator<Desc>* descAllocator, Allocator<NodeDesc>* nodeDescAllocator, uint32_t keyRange);
    ~TxMdList();

    bool ExecuteOps(Desc* desc);

    Desc* AllocateDesc(uint8_t size);
    void ResetMetrics();



private:
template<int D>
    void KeyToCoord(uint32_t key, uint8_t coord[]);

    ReturnCode Insert(uint32_t key, Desc* desc, uint8_t opid, Node*& inserted, Node*& pred, uint32_t& dim, uint32_t& pred_dim);
    ReturnCode Delete(uint32_t key, Desc* desc, uint8_t opid, Node*& deleted, Node*& pred, uint32_t& dim, uint32_t& pred_dim);
    ReturnCode Find(uint32_t key, Desc* desc, uint8_t opid);

    // bool IsNodeExist(Node* pred, Node* curr, uint8_t dp, uint8_t dc);
    bool IsNodeExist(Node* node, uint32_t key);
    void HelpOps(Desc* desc, uint8_t opid);
    bool IsSameOperation(NodeDesc* nodeDesc1, NodeDesc* nodeDesc2);
    void FinishPendingTxn(NodeDesc* nodeDesc, Desc* desc);
    bool IsNodeActive(NodeDesc* nodeDesc);
    bool IsKeyExist(NodeDesc* nodeDesc);
    bool IsNodeActive_mod(NodeDesc* nodeDesc, Desc* desc_this); //CORRECTNESS ANNOTATIONS
    bool IsKeyExist_mod(NodeDesc* nodeDesc, Desc* desc_this); //CORRECTNESS ANNOTATIONS
    void LocatePred(uint8_t coord[], Node*& pred, Node*& curr, uint32_t& dim, uint32_t& pred_dim);
    void MarkForDeletion(const std::vector<Node*>& nodes, const std::vector<Node*>& preds, const std::vector<uint32_t>& pred_dims, Desc* desc);    

    AdoptDesc* FillNewNode(Node* new_node, Node*& pred, Node*& curr, uint32_t& dim, uint32_t& pred_dim);
    void FinishInserting(Node* n, AdoptDesc* desc);

    void Traverse(Node* n, Node* parent, int dim, std::string& prefix);

    void Print();

private:
    Node* m_head;
    uint32_t m_basis;

    Allocator<Node>* m_nodeAllocator;
    Allocator<Desc>* m_descAllocator;
    Allocator<NodeDesc>* m_nodeDescAllocator;
    Allocator<AdoptDesc>* m_AdoptDescAllocator;

    ASSERT_CODE
    (
        uint32_t g_count = 0;
        uint32_t g_count_ins = 0;
        uint32_t g_count_ins_new = 0;
        uint32_t g_count_del = 0;
        uint32_t g_count_del_new = 0;
        uint32_t g_count_fnd = 0;
    )

    PERSIST_CODE
    (
        bool needPersistenceHelp(Desc* desc);
        void persistDesc(Desc* desc);
        std::atomic<uint32_t> lastTxId;
    )

    uint32_t g_count_commit = 0;
    uint32_t g_count_abort = 0;
    uint32_t g_count_fake_abort = 0;
};


#endif /* end of include guard: TRANSMDLIST_H */    
