#ifndef SETADAPTOR_H
#define SETADAPTOR_H

#include <cmath>

#include "../lftt/list/translist.h"
#include "../lftt/skiplist/transskip.h"
#include "../lftt/mdlist/txmdlist.h"
#include "../lftt/map/transmap.h"

#include "../romulus/datastructures/TMLinkedListSet.hpp"

// #include "rstm/list/rstmlist.hpp"
// #include "boosting/list/boostinglist.h"
// #include "boosting/skiplist/boostingskip.h"
#include "../common/allocator.h"
// #include "ostm/skiplist/stmskip.h"

#define RUN_COUNTER "45"

#define NODE_ALLOCATOR_NAME_PREFIX "m_nodeAllocator"
#define DESC_ALLOCATOR_NAME_PREFIX "m_descAllocator"
#define NODE_DESC_ALLOCATOR_NAME_PREFIX "m_nodeDescAllocator"
#define ADOPT_DESC_ALLOCATOR_NAME_PREFIX "m_AdoptDescAllocator"
#define MAP_SPINE_ALLOCATOR_NAME_PREFIX "m_mapSpineAllocator"
#define HEAD_ALLOCATOR_NAME_PREFIX "m_headAllocator"

#define NODE_ALLOCATOR_ALLOCATOR_NAME_PREFIX "m_nodeAllocatorAllocator"
#define DESC_ALLOCATOR_ALLOCATOR_NAME_PREFIX "m_descAllocatorAllocator"
#define NODE_DESC_ALLOCATOR_ALLOCATOR_NAME_PREFIX "m_nodeDescAllocatorAllocator"
#define ADOPT_DESC_ALLOCATOR_ALLOCATOR_NAME_PREFIX "m_AdoptDescAllocatorAllocator"
#define MAP_SPINE_ALLOCATOR_ALLOCATOR_NAME_PREFIX "m_mapSpineAllocatorAllocator"
#define HEAD_ALLOCATOR_ALLOCATOR_NAME_PREFIX "m_headAllocatorAllocator"

#define TRANS_LIST_NAME "TransList"
#define TRANS_SKIPLIST_NAME "TransSkipList"
#define TRANS_MDLIST_NAME "TransMDList"
#define TRANS_MAP_NAME "TransMap"

enum SetOpType
{
    FIND = 0,
    INSERT,
    DELETE
};

struct SetOperator
{
    uint8_t type;
    uint32_t key;
};

enum SetOpStatus
{
    LIVE = 0,
    COMMITTED,
    ABORTED
};

enum SetPersistStatus
{
    MAYBE = 0,
    IN_PROGRESS,
    PERSISTED,
};

typedef std::vector<SetOperator> SetOpArray;

template<typename T>
class SetAdaptor
{
};

template<>
class SetAdaptor<TransList>
{
public:
    SetAdaptor(uint64_t cap, uint64_t threadCount, uint32_t transSize, uint32_t keyRange, bool recovery = false)
    {
        m_threadCount = threadCount;
        cap = std::max(cap,(uint64_t)keyRange);
        if(!recovery) {
            ALLOCATOR_DRAM_ONLY_CODE
            (
                // std::cout << "USING DRAM ALLOCATOR" << std::endl;
                m_descAllocator = new Allocator<TransList::Desc>(cap * (threadCount + 1) * TransList::Desc::SizeOf(transSize), threadCount, TransList::Desc::SizeOf(transSize));
                m_nodeAllocator = new Allocator<TransList::Node>(cap * (threadCount + 1) *  sizeof(TransList::Node) * transSize, (threadCount + 1), sizeof(TransList::Node));
                m_nodeDescAllocator = new Allocator<TransList::NodeDesc>(cap * (threadCount * 3) *  sizeof(TransList::NodeDesc) * transSize, threadCount, sizeof(TransList::NodeDesc));
            )  

            ALLOCATOR_PERSISTABLE_ONLY_CODE
            (
                // std::cout << "USING MMAP ALLOCATOR" << std::endl;
                NODE_ALLOCATOR_NAME = TRANS_LIST_NAME NODE_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
                DESC_ALLOCATOR_NAME = TRANS_LIST_NAME DESC_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
                NODE_DESC_ALLOCATOR_NAME = TRANS_LIST_NAME NODE_DESC_ALLOCATOR_NAME_PREFIX RUN_COUNTER;

                NODE_ALLOCATOR_ALLOCATOR_NAME = TRANS_LIST_NAME NODE_ALLOCATOR_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
                DESC_ALLOCATOR_ALLOCATOR_NAME = TRANS_LIST_NAME DESC_ALLOCATOR_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
                NODE_DESC_ALLOCATOR_ALLOCATOR_NAME = TRANS_LIST_NAME NODE_DESC_ALLOCATOR_ALLOCATOR_NAME_PREFIX RUN_COUNTER;

                m_nodeAllocatorAllocator = new Allocator<Allocator<TransList::Node>>(NODE_ALLOCATOR_ALLOCATOR_NAME, sizeof(Allocator<Allocator<TransList::Node>>), 1, sizeof(Allocator<Allocator<TransList::Node>>));
                m_descAllocatorAllocator = new Allocator<Allocator<TransList::Desc>>(DESC_ALLOCATOR_ALLOCATOR_NAME, sizeof(Allocator<Allocator<TransList::Desc>>), 1, sizeof(Allocator<Allocator<TransList::Desc>>));
                m_nodeDescAllocatorAllocator = new Allocator<Allocator<TransList::NodeDesc>>(NODE_DESC_ALLOCATOR_ALLOCATOR_NAME, sizeof(Allocator<Allocator<TransList::NodeDesc>>), 1, sizeof(Allocator<Allocator<TransList::NodeDesc>>));

                m_nodeAllocatorAllocator->Init();
                m_descAllocatorAllocator->Init();
                m_nodeDescAllocatorAllocator->Init();

                m_nodeAllocator = new (m_nodeAllocatorAllocator->Alloc()) Allocator<TransList::Node>(NODE_ALLOCATOR_NAME, cap * (threadCount + 1) *  sizeof(TransList::Node) * transSize, (threadCount + 1), sizeof(TransList::Node));
                m_descAllocator = new (m_descAllocatorAllocator->Alloc()) Allocator<TransList::Desc>(DESC_ALLOCATOR_NAME, cap * (threadCount + 1) * TransList::Desc::SizeOf(transSize), threadCount, TransList::Desc::SizeOf(transSize));
                m_nodeDescAllocator = new (m_nodeDescAllocatorAllocator->Alloc()) Allocator<TransList::NodeDesc>(NODE_DESC_ALLOCATOR_NAME, cap * (threadCount * 3) *  sizeof(TransList::NodeDesc) * transSize, threadCount, sizeof(TransList::NodeDesc));
            )
            
            m_nodeAllocator->Init(); //for intializing head and tail
            m_list = new TransList(m_nodeAllocator, m_descAllocator, m_nodeDescAllocator);
        }else {

            ALLOCATOR_PERSISTABLE_ONLY_CODE
            (
                // std::cout << "USING MMAP ALLOCATOR" << std::endl;
                NODE_ALLOCATOR_NAME = TRANS_LIST_NAME NODE_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
                DESC_ALLOCATOR_NAME = TRANS_LIST_NAME DESC_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
                NODE_DESC_ALLOCATOR_NAME = TRANS_LIST_NAME NODE_DESC_ALLOCATOR_NAME_PREFIX RUN_COUNTER;

                NODE_ALLOCATOR_ALLOCATOR_NAME = TRANS_LIST_NAME NODE_ALLOCATOR_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
                DESC_ALLOCATOR_ALLOCATOR_NAME = TRANS_LIST_NAME DESC_ALLOCATOR_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
                NODE_DESC_ALLOCATOR_ALLOCATOR_NAME = TRANS_LIST_NAME NODE_DESC_ALLOCATOR_ALLOCATOR_NAME_PREFIX RUN_COUNTER;

                m_nodeAllocatorAllocator = new Allocator<Allocator<TransList::Node>>(NODE_ALLOCATOR_ALLOCATOR_NAME, sizeof(Allocator<Allocator<TransList::Node>>), 1, sizeof(Allocator<Allocator<TransList::Node>>));
                m_descAllocatorAllocator = new Allocator<Allocator<TransList::Desc>>(DESC_ALLOCATOR_ALLOCATOR_NAME, sizeof(Allocator<Allocator<TransList::Desc>>), 1, sizeof(Allocator<Allocator<TransList::Desc>>));
                m_nodeDescAllocatorAllocator = new Allocator<Allocator<TransList::NodeDesc>>(NODE_DESC_ALLOCATOR_ALLOCATOR_NAME, sizeof(Allocator<Allocator<TransList::NodeDesc>>), 1, sizeof(Allocator<Allocator<TransList::NodeDesc>>));

                m_nodeAllocatorAllocator->Init();
                m_descAllocatorAllocator->Init();
                m_nodeDescAllocatorAllocator->Init();

                std::cout << " after initing AllocatorAllocators" << std::endl;

                m_nodeAllocator = m_nodeAllocatorAllocator->getFirst();
                m_descAllocator = m_descAllocatorAllocator->getFirst();
                m_nodeDescAllocator = m_nodeDescAllocatorAllocator->getFirst();

                std::cout << " after calling getFirsts " << std::endl;

                // m_nodeAllocator->print();

                m_nodeAllocator->reload_pmem(NODE_ALLOCATOR_NAME);
                m_descAllocator->reload_pmem(DESC_ALLOCATOR_NAME);
                m_nodeDescAllocator->reload_pmem(NODE_DESC_ALLOCATOR_NAME); 

                std::cout << " after reloading everything " << std::endl;   

                // m_nodeAllocator->print();

                m_nodeAllocator->Init();
                m_descAllocator->Init();
                m_nodeDescAllocator->Init();                            

                m_nodeAllocator->print();
                // m_descAllocator->print();
                m_nodeDescAllocator->print();
                
                // m_nodeAllocator->Init(); //for intializing head and tail
                m_list = new TransList(m_nodeAllocator, m_descAllocator, m_nodeDescAllocator, false);

                std::cout << " after initializing list " << std::endl;   

                // transList->validate_and_recovery(threadCount, m_descAllocator);

            )

        }
        


     }

    void Init()
    {
        m_descAllocator->Init();
        m_nodeAllocator->Init();
        m_nodeDescAllocator->Init();
                // m_nodeAllocator->print();
                // m_descAllocator->print();
                // m_nodeDescAllocator->print();        
        m_list->ResetMetrics();
    }

    void Uninit(){
        // m_list->validate_and_recovery(m_threadCount, m_descAllocator);
        delete m_list;
        // static int count = 1;
        // std::cout <<" Uninit " << count++ << std::endl;
    }

NO_READ_ONLY_OPT_CODE
(
    bool ExecuteOps(const SetOpArray& ops)
    {
)

READ_ONLY_OPT_CODE
(
    bool ExecuteOps(const SetOpArray& ops, bool isReadOnly = false)
    {
)
        // static int counter = 0;
        // std::cout<< "ExecuteOps " << counter++ << std::endl;
        //TransList::Desc* desc = m_list.AllocateDesc(ops.size());
        TransList::Desc* desc = m_descAllocator->Alloc();
        desc->size = ops.size();
        desc->status = TransList::ACTIVE;
        desc->persistStatus = TransList::MAYBE;
READ_ONLY_OPT_CODE
(        
        desc->isReadOnly = isReadOnly;
)

        for(uint32_t i = 0; i < ops.size(); ++i)
        {
            desc->ops[i].type = ops[i].type; 
            desc->ops[i].key = ops[i].key; 
        }

        // printf("%p, %p, %d\n", desc, &(desc->size), desc->size);
        // std::cout << desc->size << std::endl;

        return m_list->ExecuteOps(desc);
    }

	
private:
    Allocator<TransList::Desc> *m_descAllocator;
    Allocator<TransList::Node> *m_nodeAllocator;
    Allocator<TransList::NodeDesc> *m_nodeDescAllocator;
    TransList *m_list;
    int m_threadCount;

    Allocator<Allocator<TransList::Node>> *m_nodeAllocatorAllocator;
    Allocator<Allocator<TransList::Desc>> *m_descAllocatorAllocator;
    Allocator<Allocator<TransList::NodeDesc>> *m_nodeDescAllocatorAllocator;

    const char *NODE_ALLOCATOR_NAME;// = "m_nodeAllocator" RUN_COUNTER;
    const char *DESC_ALLOCATOR_NAME;// = "m_descAllocator" RUN_COUNTER;
    const char *NODE_DESC_ALLOCATOR_NAME;// = "m_nodeDescAllocator" RUN_COUNTER;

    const char *NODE_ALLOCATOR_ALLOCATOR_NAME;// = "m_nodeAllocatorAllocator" RUN_COUNTER;
    const char *DESC_ALLOCATOR_ALLOCATOR_NAME;// = "m_descAllocatorAllocator" RUN_COUNTER;
    const char *NODE_DESC_ALLOCATOR_ALLOCATOR_NAME;// = "m_nodeDescAllocatorAllocator" RUN_COUNTER;    
};


#define NUM_LEVELS 20

template<>
class SetAdaptor<trans_skip>
{
public:
    SetAdaptor(uint64_t cap, uint64_t threadCount, uint32_t transSize, uint32_t keyRange)
        // : m_nodeAllocator(std::max(cap,(uint64_t)keyRange) * (threadCount + 1) * sizeof(node_t) * transSize * (NUM_LEVELS + 1), threadCount, (sizeof(node_t) * (NUM_LEVELS + 1)))
        // , m_descAllocator(std::max(cap,(uint64_t)keyRange) * (threadCount + 1) * Desc::SizeOf(transSize), threadCount, Desc::SizeOf(transSize))
        // , m_nodeDescAllocator(std::max(cap,(uint64_t)keyRange) * (threadCount + 1) *  sizeof(NodeDesc) * transSize * (NUM_LEVELS + 1), threadCount, (sizeof(NodeDesc) * (NUM_LEVELS + 1)))
    { 
        cap = std::max(cap,(uint64_t)keyRange);

	ALLOCATOR_DRAM_ONLY_CODE
            (
                // std::cout << "USING DRAM ALLOCATOR" << std::endl;
                m_descAllocator = new Allocator<Desc>(cap * (threadCount + 1) * Desc::SizeOf(transSize), threadCount, Desc::SizeOf(transSize));
                m_nodeAllocator = new Allocator<node_t>(cap * (threadCount + 1) *  sizeof(node_t) * transSize, (threadCount + 1), sizeof(node_t));
                m_nodeDescAllocator = new Allocator<NodeDesc>(cap * (threadCount * 3) *  sizeof(NodeDesc) * transSize, threadCount, sizeof(NodeDesc));
            ) 

        ALLOCATOR_PERSISTABLE_ONLY_CODE
        (
            // std::cout << "USING MMAP ALLOCATOR" << std::endl;
            NODE_ALLOCATOR_NAME = TRANS_SKIPLIST_NAME NODE_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
            DESC_ALLOCATOR_NAME = TRANS_SKIPLIST_NAME DESC_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
            NODE_DESC_ALLOCATOR_NAME = TRANS_SKIPLIST_NAME NODE_DESC_ALLOCATOR_NAME_PREFIX RUN_COUNTER;

            NODE_ALLOCATOR_ALLOCATOR_NAME = TRANS_SKIPLIST_NAME NODE_ALLOCATOR_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
            DESC_ALLOCATOR_ALLOCATOR_NAME = TRANS_SKIPLIST_NAME DESC_ALLOCATOR_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
            NODE_DESC_ALLOCATOR_ALLOCATOR_NAME = TRANS_SKIPLIST_NAME NODE_DESC_ALLOCATOR_ALLOCATOR_NAME_PREFIX RUN_COUNTER;

            m_nodeAllocatorAllocator = new Allocator<Allocator<node_t>>(NODE_ALLOCATOR_ALLOCATOR_NAME, sizeof(Allocator<Allocator<node_t>>), 1, sizeof(Allocator<Allocator<node_t>>));
            m_descAllocatorAllocator = new Allocator<Allocator<Desc>>(DESC_ALLOCATOR_ALLOCATOR_NAME, sizeof(Allocator<Allocator<Desc>>), 1, sizeof(Allocator<Allocator<Desc>>));
            m_nodeDescAllocatorAllocator = new Allocator<Allocator<NodeDesc>>(NODE_DESC_ALLOCATOR_ALLOCATOR_NAME, sizeof(Allocator<Allocator<NodeDesc>>), 1, sizeof(Allocator<Allocator<NodeDesc>>));

            m_nodeAllocatorAllocator->Init();
            m_descAllocatorAllocator->Init();
            m_nodeDescAllocatorAllocator->Init();

            m_nodeAllocator = new (m_nodeAllocatorAllocator->Alloc()) Allocator<node_t>(NODE_ALLOCATOR_NAME, cap * (threadCount + 1) *  sizeof(node_t) * transSize * (NUM_LEVELS + 1), (threadCount + 1), sizeof(node_t) * (NUM_LEVELS + 1));
            m_descAllocator = new (m_descAllocatorAllocator->Alloc()) Allocator<Desc>(DESC_ALLOCATOR_NAME, cap * (threadCount + 1) * Desc::SizeOf(transSize), threadCount, Desc::SizeOf(transSize));
            m_nodeDescAllocator = new (m_nodeDescAllocatorAllocator->Alloc()) Allocator<NodeDesc>(NODE_DESC_ALLOCATOR_NAME, cap * (threadCount * 3) *  sizeof(NodeDesc) * transSize * (NUM_LEVELS + 1), threadCount, sizeof(NodeDesc) * (NUM_LEVELS + 1));
        )

        m_skiplist = transskip_alloc(m_descAllocator, m_nodeDescAllocator, m_nodeAllocator);
        init_transskip_subsystem(); 
    }

    ~SetAdaptor()
    {
        // printf("~SetAdaptor()\n\r");
        // transskip_free(m_skiplist);
    }

    void Init()
    {
        m_descAllocator->Init();
        m_nodeDescAllocator->Init();
        m_nodeAllocator->Init();
        transskip_reset_metrics();
    }

    void Uninit()
    {
        // printf("uninit\n\r");
        transskip_free(m_skiplist);
        destroy_transskip_subsystem(); 
        // printf("uninit\n\r");
    }

NO_READ_ONLY_OPT_CODE
(
    bool ExecuteOps(const SetOpArray& ops)
    {
)


READ_ONLY_OPT_CODE
(
    bool ExecuteOps(const SetOpArray& ops, bool isReadOnly = false)
    {
)
        //TransList::Desc* desc = m_list.AllocateDesc(ops.size());
        Desc* desc = m_descAllocator->Alloc();
        desc->size = ops.size();
        desc->status = LIVE;
        desc->persistStatus = MAYBE;

READ_ONLY_OPT_CODE
(        
        desc->isReadOnly = isReadOnly;
)        

        for(uint32_t i = 0; i < ops.size(); ++i)
        {
            desc->ops[i].type = ops[i].type; 
            desc->ops[i].key = ops[i].key; 
        }

        return execute_ops(m_skiplist, desc);
    }

private:


    Allocator<Allocator<node_t>> *m_nodeAllocatorAllocator;
    Allocator<Allocator<Desc>> *m_descAllocatorAllocator;
    Allocator<Allocator<NodeDesc>> *m_nodeDescAllocatorAllocator;


    Allocator<node_t>* m_nodeAllocator;
    Allocator<Desc>* m_descAllocator;
    Allocator<NodeDesc>* m_nodeDescAllocator;
    trans_skip* m_skiplist;

    const char *NODE_ALLOCATOR_NAME;// = "m_nodeAllocator" RUN_COUNTER;
    const char *DESC_ALLOCATOR_NAME;// = "m_descAllocator" RUN_COUNTER;
    const char *NODE_DESC_ALLOCATOR_NAME;// = "m_nodeDescAllocator" RUN_COUNTER;

    const char *NODE_ALLOCATOR_ALLOCATOR_NAME;// = "m_nodeAllocatorAllocator" RUN_COUNTER;
    const char *DESC_ALLOCATOR_ALLOCATOR_NAME;// = "m_descAllocatorAllocator" RUN_COUNTER;
    const char *NODE_DESC_ALLOCATOR_ALLOCATOR_NAME;// = "m_nodeDescAllocatorAllocator" RUN_COUNTER;

};

template<>
class SetAdaptor<TxMdList>
{
public:
    SetAdaptor(uint64_t cap, uint64_t threadCount, uint32_t transSize, uint32_t keyRange)
        // : m_descAllocator(std::max(cap,(uint64_t)keyRange) * threadCount * TxMdList::Desc::SizeOf(transSize), threadCount, TxMdList::Desc::SizeOf(transSize))
        // , m_nodeAllocator(std::max(cap,(uint64_t)keyRange) * threadCount *  sizeof(TxMdList::Node) * transSize, threadCount, sizeof(TxMdList::Node))
        // , m_nodeDescAllocator(std::max(cap,(uint64_t)keyRange) * threadCount *  sizeof(TxMdList::NodeDesc) * transSize, threadCount, sizeof(TxMdList::NodeDesc))
        // , m_AdoptDescAllocator(std::max(cap,(uint64_t)keyRange) * threadCount *  sizeof(TxMdList::AdoptDesc) * transSize, threadCount, sizeof(TxMdList::AdoptDesc))
        // , m_list(&m_nodeAllocator, &m_descAllocator, &m_nodeDescAllocator, &m_AdoptDescAllocator, keyRange)
    { 
        cap = std::max(cap,(uint64_t)keyRange);

		ALLOCATOR_DRAM_ONLY_CODE
            (
                // std::cout << "USING DRAM ALLOCATOR" << std::endl;
                m_descAllocator = new Allocator<TxMdList::Desc>(cap * (threadCount + 1) * TxMdList::Desc::SizeOf(transSize), threadCount, TxMdList::Desc::SizeOf(transSize));
                m_nodeAllocator = new Allocator<TxMdList::Node>(cap * (threadCount + 1) *  sizeof(TxMdList::Node) * transSize, (threadCount + 1), sizeof(TxMdList::Node));
                m_nodeDescAllocator = new Allocator<TxMdList::NodeDesc>(cap * (threadCount * 3) *  sizeof(TxMdList::NodeDesc) * transSize, threadCount, sizeof(TxMdList::NodeDesc));
            )  

        ALLOCATOR_PERSISTABLE_ONLY_CODE
        (
            // std::cout << "USING MMAP ALLOCATOR" << std::endl;
            NODE_ALLOCATOR_NAME = TRANS_MDLIST_NAME NODE_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
            DESC_ALLOCATOR_NAME = TRANS_MDLIST_NAME DESC_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
            NODE_DESC_ALLOCATOR_NAME = TRANS_MDLIST_NAME NODE_DESC_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
            ADOPT_DESC_ALLOCATOR_NAME = TRANS_MDLIST_NAME ADOPT_DESC_ALLOCATOR_NAME_PREFIX RUN_COUNTER;

            NODE_ALLOCATOR_ALLOCATOR_NAME = TRANS_MDLIST_NAME NODE_ALLOCATOR_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
            DESC_ALLOCATOR_ALLOCATOR_NAME = TRANS_MDLIST_NAME DESC_ALLOCATOR_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
            NODE_DESC_ALLOCATOR_ALLOCATOR_NAME = TRANS_MDLIST_NAME NODE_DESC_ALLOCATOR_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
            ADOPT_DESC_ALLOCATOR_ALLOCATOR_NAME = TRANS_MDLIST_NAME ADOPT_DESC_ALLOCATOR_ALLOCATOR_NAME_PREFIX RUN_COUNTER;

            m_nodeAllocatorAllocator = new Allocator<Allocator<TxMdList::Node>>(NODE_ALLOCATOR_ALLOCATOR_NAME, sizeof(Allocator<Allocator<TxMdList::Node>>), 1, sizeof(Allocator<Allocator<TxMdList::Node>>));
            m_descAllocatorAllocator = new Allocator<Allocator<TxMdList::Desc>>(DESC_ALLOCATOR_ALLOCATOR_NAME, sizeof(Allocator<Allocator<TxMdList::Desc>>), 1, sizeof(Allocator<Allocator<TxMdList::Desc>>));
            m_nodeDescAllocatorAllocator = new Allocator<Allocator<TxMdList::NodeDesc>>(NODE_DESC_ALLOCATOR_ALLOCATOR_NAME, sizeof(Allocator<Allocator<TxMdList::NodeDesc>>), 1, sizeof(Allocator<Allocator<TxMdList::NodeDesc>>));
            m_AdoptDescAllocatorAllocator = new Allocator<Allocator<TxMdList::AdoptDesc>>(ADOPT_DESC_ALLOCATOR_ALLOCATOR_NAME, sizeof(Allocator<Allocator<TxMdList::AdoptDesc>>), 1, sizeof(Allocator<Allocator<TxMdList::AdoptDesc>>));

            m_nodeAllocatorAllocator->Init();
            m_descAllocatorAllocator->Init();
            m_nodeDescAllocatorAllocator->Init();
            m_AdoptDescAllocatorAllocator->Init();


            m_nodeAllocator = new (m_nodeAllocatorAllocator->Alloc()) Allocator<TxMdList::Node>(NODE_ALLOCATOR_NAME, cap * (threadCount * 2) *  sizeof(TxMdList::Node) * transSize, (threadCount), sizeof(TxMdList::Node));
            m_descAllocator = new (m_descAllocatorAllocator->Alloc()) Allocator<TxMdList::Desc>(DESC_ALLOCATOR_NAME, cap * (threadCount * 3) * TxMdList::Desc::SizeOf(transSize), threadCount, TxMdList::Desc::SizeOf(transSize));
            m_nodeDescAllocator = new (m_nodeDescAllocatorAllocator->Alloc()) Allocator<TxMdList::NodeDesc>(NODE_DESC_ALLOCATOR_NAME, cap * (threadCount * 2) *  sizeof(TxMdList::NodeDesc) * transSize, threadCount, sizeof(TxMdList::NodeDesc));
            // m_AdoptDescAllocator = new (m_AdoptDescAllocatorAllocator->Alloc()) Allocator<TxMdList::AdoptDesc>(ADOPT_DESC_ALLOCATOR_NAME, cap * (threadCount) *  sizeof(TxMdList::AdoptDesc) * transSize, threadCount, sizeof(TxMdList::AdoptDesc));
            m_AdoptDescAllocator = nullptr;//new (m_AdoptDescAllocatorAllocator->Alloc()) Allocator<TxMdList::AdoptDesc>(ADOPT_DESC_ALLOCATOR_NAME, cap * (threadCount) *  sizeof(TxMdList::AdoptDesc) * transSize, threadCount, sizeof(TxMdList::AdoptDesc));
        )

        m_list = new TxMdList(m_nodeAllocator, m_descAllocator, m_nodeDescAllocator, keyRange);

    }

    void Init()
    {
        m_descAllocator->Init();
        m_nodeAllocator->Init();
        m_nodeDescAllocator->Init();
        // m_AdoptDescAllocator->Init();
        m_list->ResetMetrics();
    }

    void Uninit(){
        delete m_list;
    }

NO_READ_ONLY_OPT_CODE
(
    bool ExecuteOps(const SetOpArray& ops)
    {
)

READ_ONLY_OPT_CODE
(
    bool ExecuteOps(const SetOpArray& ops, bool isReadOnly = false)
    {
)
        //TransList::Desc* desc = m_list.AllocateDesc(ops.size());
        TxMdList::Desc* desc = m_descAllocator->Alloc();
        desc->size = ops.size();
        desc->status = TxMdList::ACTIVE;
        desc->persistStatus = TxMdList::MAYBE;
READ_ONLY_OPT_CODE
(        
        desc->isReadOnly = isReadOnly;
)
        for(uint32_t i = 0; i < ops.size(); ++i)
        {
            desc->ops[i].type = ops[i].type; 
            desc->ops[i].key = ops[i].key; 
        }

        return m_list->ExecuteOps(desc);
    }

private:

    Allocator<Allocator<TxMdList::Node>> *m_nodeAllocatorAllocator;
    Allocator<Allocator<TxMdList::Desc>> *m_descAllocatorAllocator;
    Allocator<Allocator<TxMdList::NodeDesc>> *m_nodeDescAllocatorAllocator;
    Allocator<Allocator<TxMdList::AdoptDesc>> *m_AdoptDescAllocatorAllocator;


    Allocator<TxMdList::Desc>* m_descAllocator;
    Allocator<TxMdList::Node>* m_nodeAllocator;
    Allocator<TxMdList::NodeDesc>* m_nodeDescAllocator;
    Allocator<TxMdList::AdoptDesc>* m_AdoptDescAllocator;



    const char *NODE_ALLOCATOR_NAME;// = "m_nodeAllocator" RUN_COUNTER;
    const char *DESC_ALLOCATOR_NAME;// = "m_descAllocator" RUN_COUNTER;
    const char *NODE_DESC_ALLOCATOR_NAME;// = "m_nodeDescAllocator" RUN_COUNTER;
    const char *ADOPT_DESC_ALLOCATOR_NAME;// = "m_nodeDescAllocator" RUN_COUNTER;

    const char *NODE_ALLOCATOR_ALLOCATOR_NAME;// = "m_nodeAllocatorAllocator" RUN_COUNTER;
    const char *DESC_ALLOCATOR_ALLOCATOR_NAME;// = "m_descAllocatorAllocator" RUN_COUNTER;
    const char *NODE_DESC_ALLOCATOR_ALLOCATOR_NAME;// = "m_nodeDescAllocatorAllocator" RUN_COUNTER;
    const char *ADOPT_DESC_ALLOCATOR_ALLOCATOR_NAME;// = "m_nodeDescAllocator" RUN_COUNTER;

    TxMdList* m_list;
};


template<typename T>
class MapAdaptor
{
};

enum MapOpType
{
    MAP_FIND = 0,
    MAP_INSERT,
    MAP_DELETE,
    MAP_UPDATE
};

struct MapOperator
{
    uint8_t type;
    uint32_t key;
    uint32_t value;
    uint32_t expected;
    uint32_t threadId;
};

enum MapOpStatus
{
    MAP_ACTIVE = 0,
    MAP_COMMITTED,
    MAP_ABORTED
};

typedef std::vector<MapOperator> MapOpArray;

template<>
class MapAdaptor<TransMap>
{
public:


    MapAdaptor(uint64_t cap, uint64_t threadCount, uint32_t transSize, uint32_t keyRange)
    { 
        cap = std::max(cap,(uint64_t)keyRange);
        int mapHeadSize				=std::pow(2, ((int)std::ceil(std::log2(cap))));
        cap = mapHeadSize;// = cap;
        ALLOCATOR_DRAM_ONLY_CODE
        (
            // std::cout << "USING DRAM ALLOCATOR" << std::endl;
            m_descAllocator = new Allocator<TransMap::Desc>(cap * (8192) * TransMap::Desc::SizeOf(transSize), threadCount, TransMap::Desc::SizeOf(transSize));
            m_nodeAllocator = new Allocator<TransMap::DataNode>(cap * (3072) *  sizeof(TransMap::DataNode) * transSize, (threadCount), sizeof(TransMap::DataNode));
            m_nodeDescAllocator = new Allocator<TransMap::NodeDesc>(cap * (8192) *  sizeof(TransMap::NodeDesc) * transSize, threadCount, sizeof(TransMap::NodeDesc) );
            m_mapSpineAllocator = new Allocator<TransMap::MapSpine>(cap * (threadCount * 4) *  sizeof(TransMap::MapSpine) * transSize, threadCount, sizeof(TransMap::MapSpine));
            m_headAllocator = new Allocator<void **>(mapHeadSize * threadCount * sizeof(void *), threadCount, sizeof(void *));
        )

        ALLOCATOR_PERSISTABLE_ONLY_CODE
        (
            // std::cout << "USING MMAP ALLOCATOR" << std::endl;
            NODE_ALLOCATOR_NAME = TRANS_MAP_NAME NODE_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
            DESC_ALLOCATOR_NAME = TRANS_MAP_NAME DESC_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
            NODE_DESC_ALLOCATOR_NAME = TRANS_MAP_NAME NODE_DESC_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
            MAP_SPINE_ALLOCATOR_NAME = TRANS_MAP_NAME MAP_SPINE_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
            HEAD_ALLOCATOR_NAME = TRANS_MAP_NAME HEAD_ALLOCATOR_NAME_PREFIX RUN_COUNTER;            

            NODE_ALLOCATOR_ALLOCATOR_NAME = TRANS_MAP_NAME NODE_ALLOCATOR_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
            DESC_ALLOCATOR_ALLOCATOR_NAME = TRANS_MAP_NAME DESC_ALLOCATOR_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
            NODE_DESC_ALLOCATOR_ALLOCATOR_NAME = TRANS_MAP_NAME NODE_DESC_ALLOCATOR_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
            MAP_SPINE_ALLOCATOR_ALLOCATOR_NAME = TRANS_MAP_NAME MAP_SPINE_ALLOCATOR_ALLOCATOR_NAME_PREFIX RUN_COUNTER;
            HEAD_ALLOCATOR_ALLOCATOR_NAME = TRANS_MAP_NAME HEAD_ALLOCATOR_ALLOCATOR_NAME_PREFIX RUN_COUNTER;              

            m_nodeAllocatorAllocator = new Allocator<Allocator<TransMap::DataNode>>(NODE_ALLOCATOR_ALLOCATOR_NAME, sizeof(Allocator<Allocator<TransMap::DataNode>>), 1, sizeof(Allocator<Allocator<TransMap::DataNode>>));
            m_descAllocatorAllocator = new Allocator<Allocator<TransMap::Desc>>(DESC_ALLOCATOR_ALLOCATOR_NAME, sizeof(Allocator<Allocator<TransMap::Desc>>), 1, sizeof(Allocator<Allocator<TransMap::Desc>>));
            m_nodeDescAllocatorAllocator = new Allocator<Allocator<TransMap::NodeDesc>>(NODE_DESC_ALLOCATOR_ALLOCATOR_NAME, sizeof(Allocator<Allocator<TransMap::NodeDesc>>), 1, sizeof(Allocator<Allocator<TransMap::NodeDesc>>));
            m_mapSpineAllocatorAllocator = new Allocator<Allocator<TransMap::MapSpine>>(MAP_SPINE_ALLOCATOR_ALLOCATOR_NAME, sizeof(Allocator<Allocator<TransMap::MapSpine>>), 1, sizeof(Allocator<Allocator<TransMap::MapSpine>>));
            m_headAllocatorAllocator = new Allocator<Allocator<void **>>(HEAD_ALLOCATOR_ALLOCATOR_NAME, sizeof(Allocator<Allocator<void **>>), 1, sizeof(Allocator<Allocator<void **>>));

            m_nodeAllocatorAllocator->Init();
            m_descAllocatorAllocator->Init();
            m_nodeDescAllocatorAllocator->Init();
            m_mapSpineAllocatorAllocator->Init();
            m_headAllocatorAllocator->Init();


            m_descAllocator = new Allocator<TransMap::Desc>(DESC_ALLOCATOR_NAME, cap * (threadCount * 2) * TransMap::Desc::SizeOf(transSize), threadCount, TransMap::Desc::SizeOf(transSize));
            m_nodeAllocator = new Allocator<TransMap::DataNode>(NODE_ALLOCATOR_NAME, cap * (threadCount * 32) *  sizeof(TransMap::DataNode) * transSize, (threadCount), sizeof(TransMap::DataNode));
            m_nodeDescAllocator = new Allocator<TransMap::NodeDesc>(NODE_DESC_ALLOCATOR_NAME, cap * (threadCount * 3) *  sizeof(TransMap::NodeDesc) * transSize, threadCount, sizeof(TransMap::NodeDesc));
            m_mapSpineAllocator = new Allocator<TransMap::MapSpine>(MAP_SPINE_ALLOCATOR_NAME, cap * (threadCount * 16) *  sizeof(TransMap::MapSpine) * transSize, threadCount, sizeof(TransMap::MapSpine));
            m_headAllocator = new Allocator<void **>(HEAD_ALLOCATOR_NAME, mapHeadSize * threadCount * sizeof(void *), threadCount, sizeof(void *));

        )
        m_headAllocator->Init();
        m_map = new TransMap(m_headAllocator, m_nodeAllocator, m_descAllocator, m_nodeDescAllocator, m_mapSpineAllocator, cap, threadCount);


    }

    void Init()
    {
        m_descAllocator->Init();
        m_nodeAllocator->Init();
        m_nodeDescAllocator->Init();
        m_mapSpineAllocator->Init();
        // m_map.ResetMetrics();
    }

    void Uninit(){
        delete m_map;
    }

NO_READ_ONLY_OPT_CODE
(
    bool ExecuteOps(const MapOpArray& ops, int threadId)
    {
)

READ_ONLY_OPT_CODE
(
    bool ExecuteOps(const MapOpArray& ops, int threadId, bool isReadOnly = false)
    {
)
        //TransList::Desc* desc = m_list.AllocateDesc(ops.size());
        TransMap::Desc* desc = m_descAllocator->Alloc();
        desc->size = ops.size();
        desc->status = TransMap::MAP_ACTIVE;
        desc->persistStatus = TransMap::MAYBE;

READ_ONLY_OPT_CODE
(        
        desc->isReadOnly = isReadOnly;
)   

        for(uint32_t i = 0; i < ops.size(); ++i)
        {
            desc->ops[i].type = ops[i].type; 
            desc->ops[i].key = ops[i].key; 
            desc->ops[i].value = ops[i].value;
        }

        return m_map->ExecuteOps(desc, threadId);
    }

    bool ExecuteOpsTATP(const MapOpArray& ops, int threadId)
    {
        //TransList::Desc* desc = m_list.AllocateDesc(ops.size());
        TransMap::Desc* desc = m_descAllocator->Alloc();
        desc->size = ops.size();
        desc->status = TransMap::MAP_ACTIVE;
        desc->persistStatus = TransMap::MAYBE;

 
        desc->ops[0].type = ops[0].type; 
        desc->ops[0].key = ops[0].key; 
        desc->ops[0].value = ops[0].value;

        return m_map->ExecuteOpsTATPBenchmark(desc, threadId);
    }    

private:
    Allocator<void**> *m_headAllocator;
    Allocator<TransMap::Desc> *m_descAllocator;
    Allocator<TransMap::DataNode> *m_nodeAllocator;
    Allocator<TransMap::NodeDesc> *m_nodeDescAllocator;
    Allocator<TransMap::MapSpine> *m_mapSpineAllocator;


    Allocator<Allocator<TransMap::DataNode>> *m_nodeAllocatorAllocator;
    Allocator<Allocator<TransMap::Desc>> *m_descAllocatorAllocator;
    Allocator<Allocator<TransMap::NodeDesc>> *m_nodeDescAllocatorAllocator;
    Allocator<Allocator<TransMap::MapSpine>> *m_mapSpineAllocatorAllocator;
    Allocator<Allocator<void **>> *m_headAllocatorAllocator;    

    const char *NODE_ALLOCATOR_NAME;// = "m_nodeAllocator" RUN_COUNTER;
    const char *DESC_ALLOCATOR_NAME;// = "m_descAllocator" RUN_COUNTER;
    const char *NODE_DESC_ALLOCATOR_NAME;// = "m_nodeDescAllocator" RUN_COUNTER;
    const char *MAP_SPINE_ALLOCATOR_NAME;
    const char *HEAD_ALLOCATOR_NAME;

    const char *NODE_ALLOCATOR_ALLOCATOR_NAME;// = "m_nodeAllocatorAllocator" RUN_COUNTER;
    const char *DESC_ALLOCATOR_ALLOCATOR_NAME;// = "m_descAllocatorAllocator" RUN_COUNTER;
    const char *NODE_DESC_ALLOCATOR_ALLOCATOR_NAME;// = "m_nodeDescAllocatorAllocator" RUN_COUNTER; 
    const char *MAP_SPINE_ALLOCATOR_ALLOCATOR_NAME;
    const char *HEAD_ALLOCATOR_ALLOCATOR_NAME;    

    TransMap* m_map;
};




#endif /* end of include guard: SETADAPTOR_H */
