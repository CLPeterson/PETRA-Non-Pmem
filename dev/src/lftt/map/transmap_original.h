#ifndef TRANSMAP_H
#define TRANSMAP_H

#include <cstdint>
#include <vector>
#include "../../common/assert.h"
#include "../../common/allocator.h"
#include "../../durabletxn/dtx.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h> //note: change wfhm assert(cond) to transmap Assert(cond, message)?
#include <iostream>
#include <iomanip>

#include <cmath>

#include "../../common/vsv.h" //CORRECTNESS ANNOTATIONS

#define HASH unsigned int
#define KEY_SIZE 32
#define toHash 5
#ifndef SUB_POW
	#define SUB_POW 6 //TODO:note subspine size
#endif

#define SUB_SIZE 64 // 2 ^ 6 POW(SUB_POW)

#define MAX_CAS_FAILURE 10

// template <class KEY, class VALUE>//, typename _tMemory>
#define KEY uint32_t
#define VALUE uint32_t

class TransMap
{
public:

	enum OpStatus
	{
	    MAP_ACTIVE = 0,
	    MAP_COMMITTED,
	    MAP_ABORTED
	};

    enum PersistStatus
    {
        MAYBE = 0,
        IN_PROGRESS,
        PERSISTED,
    };   	

	enum OpType
	{
	    MAP_FIND = 0,
	    MAP_INSERT,
	    MAP_DELETE,
	    MAP_UPDATE
	};

	struct Operator
	{
	    uint8_t type;
	    uint32_t key;
	    uint32_t value;
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

	struct MapSpine
    {
		char spine[SUB_SIZE];
    };

    typedef struct {
		union{
			HASH hash;
			void* next;
		};
		
		VALUE value;

		NodeDesc* nodeDesc;
	}DataNode;       

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
#ifdef useThreadWatch
	HASH * /* volatile  */ Thread_watch;
#endif
#ifdef useVectorPool

    void ***Thread_pool_vector;
	int *Thread_pool_vector_size;
#endif

	// void **Thread_pool_stack;
	// void **Thread_spines;
	
	int MAIN_SIZE;
	int MAIN_POW;
		
	int Threads; 
	
	void* /* volatile  */*head;
	/* volatile  */ unsigned int elements;
	#ifdef SPINE_COUNT
		/* volatile  */ unsigned int spine_elements;
	#endif


	TransMap(Allocator<void**>* headAllocator, Allocator<DataNode>* nodeAllocator, Allocator<TransMap::Desc>* descAllocator, Allocator<TransMap::NodeDesc>* nodeDescAllocator,
	Allocator<MapSpine>* _mapSpineAllocator, uint64_t initalPowerOfTwo, uint64_t numThreads, bool newMap = true)
	    : m_headAllocator(headAllocator), 
		m_nodeAllocator(nodeAllocator), 
        m_descAllocator(descAllocator)
	    , m_nodeDescAllocator(nodeDescAllocator)
		, m_mapSpineAllocator(_mapSpineAllocator)
		{
			MAIN_SIZE				=TransMap::POW(((int)std::ceil(std::log2(initalPowerOfTwo))));
			MAIN_POW				=((int)std::ceil(std::log2(initalPowerOfTwo)));
			Threads					=numThreads; assert(Threads>0);
			lastTxId.store(0);
			if(newMap) {
				head = (void **)m_headAllocator->Alloc();
			}else {
ALLOCATOR_PERSISTABLE_ONLY_CODE
(        
				head = (void **)m_headAllocator->getFirst();
)
			}
			// head					=(void* /* volatile  */ *)	calloc(MAIN_SIZE,sizeof(void */* volatile  */));

	#ifdef useThreadWatch
			Thread_watch				=(HASH * /* volatile  */ )	calloc(Threads,sizeof(HASH));
	#endif
			// Thread_pool_stack			=(void **)			calloc(Threads,sizeof(void *));
	#ifdef useVectorPool
			Thread_pool_vector			=(void ***)			calloc(Threads,sizeof(void **));
			Thread_pool_vector_size		=(int *)			calloc(Threads,sizeof(int));
			for(int i=0; i<Threads; i++){
				Thread_pool_vector[i]=(void **)		calloc(10,sizeof(void *));
				Thread_pool_vector_size[i]=10;
			}
	#endif
			
			elements				=0;
			#ifdef SPINE_COUNT
		 		spine_elements=0;
		 	#endif
			//v_capacity=MAIN_SIZE;
			#ifdef DEBUGPRINTS_MARK
			printf("DEBUGPRINTS_MARK output enabled\n");
			#endif
			//printf("initialized!\n\r");
		}

~TransMap()
	{
		printf("Total commit %u, abort (total/fake) %u/%u\n", g_count_commit, g_count_abort, g_count_fake_abort);

		FILE* pfile = fopen("output.txt", "a"); //CORRECTNESS ANNOTATIONS
		fprintf(pfile, "%u %u\n", g_count_commit, g_count_abort); //CORRECTNESS ANNOTATIONS
		fclose(pfile); //CORRECTNESS ANNOTATIONS

	}        

	//TODO UPDATE HASH FUNCTION
	/**
	 Hash function must only reorder bits! it must be 1:1
	 **/
	inline HASH HASH_KEY(KEY k) {

#if toHash==1
		HASH x=k;
		//XY
		register unsigned int y = 0x55555555;

		x = (((x >> 1) & y) | ((x & y) << 1));
		y = 0x33333333;
		x = (((x >> 2) & y) | ((x & y) << 2));
		y = 0x0f0f0f0f;
		x = (((x >> 4) & y) | ((x & y) << 4));
		y = 0x00ff00ff;
		x = (((x >> 8) & y) | ((x & y) << 8));
		return x;
#elif toHash==2

		unsigned int key=(unsigned int)k;
		key = ~key + (key << 15); // key = (key << 15) - key - 1;
		key = key ^ (key >> 12);
		key = key + (key << 2);
		key = key ^ (key >> 4);
		key = key * 2057; // key = (key + (key << 3)) + (key << 11);
		key = key ^ (key >> 16);
		return key;
#elif toHash==3
		return (((k & 0xF0000000) >> 20)+((k & 0x0F000000) >> 20)+((k & 0x00F00000) >> 20)+((k & 0x000F0000) >> 04)+
				((k & 0x0000F000) << 16)+((k & 0x00000F00) << 16)+((k & 0x000000F0) << 16)+((k & 0x0000000F) << 16));
#elif toHash==4
		return ((k & 0xFFFF0000) >> 16)+((k & 0x0000FFFF) << 16);
#elif toHash==5
		return std::hash<KEY>{}(k);


#else
		return (HASH) k;
#endif

	}

    bool ExecuteOps(Desc* desc, int threadId);
	bool ExecuteOpsDBBenchmark(Desc* desc, int threadId);
	bool ExecuteOpsTATPBenchmark(Desc* desc, int threadId);

	//////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////Bit Marking Functions//////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////	
		
	//SPINE NODE BIT MARK
	inline bool isSpine(void *p){
		/*if(p==NULL)
			return false;
		else*/
			return  (((unsigned long)(p)) & 1);
	}
	inline void * mark_spine(void * /* volatile  */*s){
		if(s==NULL)
			return NULL;
		else
			return (void *) ( ( (unsigned long)((void *)s)) |1);
	}
	inline void* /* volatile  */ *unmark_spine(void *s){
		return  (void* /* volatile  */ *)( ( (unsigned long)s) & ~3);//3 instead of one in case a spine was marked, less overhead then checking for a mark on a spine
	}  

	//DATA NODE BIT MARK
	inline DataNode* unmark_data(void *s){
		return (DataNode *) ( ( (unsigned long)((void *)s)) & ~2);
	}
	inline void* mark_data(void *s){
		return  (void *)( ( (unsigned long)s) | 2);
	}
	inline bool isMarkedData(void *p){
		return  (((unsigned long)(p)) & 2);
	}

	inline void mark_data_node(void * /* volatile  */*s, int pos){
		#ifdef DEBUGPRINTS_MARK
		printf("Node was marked!\n");
		#endif
		__sync_fetch_and_or(&(s[pos]),2);
	}
	
	inline int getMAINPOS(HASH hash){
		return hash&(MAIN_SIZE-1);
	};

	inline VALUE GetValue(NodeDesc* oldCurrDesc)
	{
		return oldCurrDesc->desc->ops[oldCurrDesc->opid].value;
	}


	inline int POW(int pow){
		int res = 1;
		if(pow==0) return 1;
		else{
			while(pow!=0){
				res=res*2;
				pow--;
			}
		}
		return res;
	}



inline bool Insert(Desc* desc, uint8_t opid, KEY k, VALUE v, int T)
{
    //inserted = NULL;
	// printf("Insert %d begins!\n\r", k);

    NodeDesc* nodeDesc = new(m_nodeDescAllocator->Alloc()) NodeDesc(desc, opid);
	// printf("Insert %d after NodeDesc alloc!\n\r", k);

	HASH hash=HASH_KEY(k);//reorders the bits in the key to more evenly distribute the bits
	#ifdef useThreadWatch
		Thread_watch[T]=hash;//Puts the hash in the watchlist
	#endif
	
	//Allocates a bucket, then stores the value, key and hash into it
    // DataNode *temp_bucket=(DataNode *) new(m_nodeAllocator->Alloc()) DataNode();
    DataNode *temp_bucket = Allocate_Node(v,hash,T, nodeDesc);
	// printf("Insert %d after Allocate_Node!\n\r", k);

	#ifdef DEBUG
	assert(temp_bucket!=NULL);
	#endif
	
	bool res=putIfAbsent_main(desc, hash,temp_bucket,T, nodeDesc);
	// printf("Insert %d after putIfAbsent_main!\n\r", k);
	// if(!res){
	// 	Free_Node_Stack(temp_bucket, T);
	// }
		
	#ifdef useThreadWatch
		Thread_watch[T]=0;//Removes the hash from the watchlist
	#endif	
	//printf("Insert finishes!\n\r");
	return res;
}


inline bool putIfAbsent_main(Desc* desc, HASH hash,DataNode *temp_bucket, int T, NodeDesc* nodeDesc){
	// printf("putIfAbsent_main begins!\n\r");
	
	//This count bounds the number of times the thread will loop as a result of CAS failure.
	int cas_fail_count=0;
insert_main:
	//Determines the position to insert at by examining the "M" right most bits
	int pos=getMAINPOS(hash);//&(MAIN_SIZE-1));
	#ifdef DEBUG
	assert(pos >=0 && pos <MAIN_SIZE);//Check to make sure position is valid
	#endif

	void *node=getNodeRaw(head,pos);//Gets the pointer value at the position of interest
	//Examining Main Spine First
	while(true){
		
		if(cas_fail_count>=MAX_CAS_FAILURE){//Checks to see if it failed the CAS to many times
			mark_data_node(head,pos);//If it has then it marks that node
				#ifdef DEBUGPRINTS_MARK
				printf("Marked a Node--Main\n");
				#endif
		}
		
		void *node2;
		if(node==NULL){//See Logic Above
			if( (node2=replace_node(head, pos, NULL, temp_bucket))==NULL){
				increment_size();//Increments the size of table
				return true;
			}	
			else{ //NOTE: don't need to check isKeyExist for a spine node, because they are never deleted under any circumstances
				if( /*node2!=NULL &&*/ isSpine(node2)){//If it is a Spine then continue, we don't know if the key was updated.
					return putIfAbsent_sub(desc,unmark_spine(node2), temp_bucket, T, nodeDesc);
				}
				else{
					cas_fail_count++;
					node=getNodeRaw(head,pos);//Gets the pointer value at the position of interest

					continue;
				}
				
			}
		}
		else if(isSpine(node)){//Check the Sub Spines
			return putIfAbsent_sub(desc,unmark_spine(node), temp_bucket, T, nodeDesc);
			
		}
		else if(isMarkedData(node)){//Force Expand The table because someone could not pass the cas
		    #ifdef DEBUGPRINTS
		    printf("marked found on main!\n");
		    #endif
		    node=forceExpandTable(T,head,pos,unmark_data(node), MAIN_POW);
			return putIfAbsent_sub(desc,unmark_spine(node), temp_bucket, T, nodeDesc);
		}
		else{//It is a Data Node
			#ifdef DEBUG
			if( ((DataNode *)node)->hash==0){
				printf("Zero Hash!");
			}
			#endif
			// if( ((DataNode *)node)->hash==temp_bucket->hash && IsKeyExist( ((DataNode *)node)->nodeDesc ) ){//It is a key match
			// 	//if(  ) //( ((DataNode *)node), temp_bucket->key, temp_bucket->value ) )

			// 		return false;
			// }
			if( ((DataNode *)node)->hash==temp_bucket->hash )
			{
				NodeDesc* oldCurrDesc = ((DataNode *)node)->nodeDesc;

				FinishPendingTxn(oldCurrDesc, desc, T);

	            if(IsSameOperation(oldCurrDesc, nodeDesc))
	            {
	                return true;
	            }

	            // the key that we wanted to insert is already in there, so fail
	            //if(IsKeyExist(oldCurrDesc))
				if(IsKeyExist_mod(oldCurrDesc, desc))
	            	return false;
	            else // the key they wanted to insert, isn't logically in the table so we can insert
            	{
            		//goto noMatch_putMain;
            		NodeDesc* currDesc = ((DataNode *)node)->nodeDesc;

	                if(desc->status != MAP_ACTIVE)
	                {
	                    return false;
	                }

	                //if(currDesc == oldCurrDesc)
	                {
	                    //Update desc to logically add the key to the table since it's already physically there
	                    currDesc = __sync_val_compare_and_swap(&((DataNode *)node)->nodeDesc, oldCurrDesc, nodeDesc);

	                    // If the CAS is successful, then the value before the CAS must have been oldCurrDesc which is returned
	                    // to currDesc leading to a successful comparison in the if statement below
	                    if(currDesc == oldCurrDesc)
	                    {
	                        ASSERT_CODE
	                            (
	                             __sync_fetch_and_add(&g_count_ins, 1);
	                            );

	                        //return true; 
	                        // NOTE: the following may break wait-freedom, but not lock-freedom
	                        // if the logically removed node has a different value, then CAS in our new value
	                        if ( ((DataNode *)node)->value != temp_bucket->value )
	                        	if(__sync_bool_compare_and_swap( &((DataNode *)node)->value, ((DataNode *)node)->value, ((DataNode* )temp_bucket)->value ) )
	                        		return true;
	                        	else // cas failed so retry
	                        		goto insert_main;
	                        else // had the same value so all we had to do was update the descriptor and return
	                        	return true;
	                    }
	                    else
	                    	goto insert_main;
	                }
            	}

			}
			else{//Create a Spine
				//Allocate Spine will return true if it succeded, and false if it failed.
				//See Below for functionality.
			//noMatch_putMain:

				bool res=Allocate_Spine(T, head,pos,(DataNode *)node,temp_bucket, MAIN_POW);
				if(res){
					increment_size();//Increments Size
					return true;
				}
				else{
					cas_fail_count++;
					node=getNodeRaw(head,pos);//Gets the pointer value at the position of interest
					continue;
				}
			}//End Else Create Spine
		}//End Else, Data Node
	}//End While Loop
	//printf("putIfAbsent_main returns false!\n\r");
	return false;
 }//End Put Main

/**
See Put_Main for logic Discription, this section is vary similar to put_main, but instead allows multiple level traversal
If you desire 3 or more different lengths of memory array then copy put_main, 
replace MAIN_POW/MAIN_SIZE with correct values, then change put_main's calls to put subs to the copied function

**DONT FORGET: to do get/delete as well
*/
inline bool putIfAbsent_sub(Desc* desc,void* /* volatile  */* local, DataNode *temp_bucket, int T, NodeDesc* nodeDesc){
	//printf("putIfAbsent_sub begins!\n\r");
	insert_sub:
		HASH h=(temp_bucket->hash)>>MAIN_POW;//Shifts the hash to move the siginifcant bits to the right most position
	for(int right=MAIN_POW; right<KEY_SIZE; right+=SUB_POW){
		int pos=h&(SUB_SIZE-1);//Gets the sig bits from the hash
		#ifdef DEBUG
		assert(pos >=0 && pos <SUB_SIZE);//Check to make sure pos is valid
		#endif
		h=h>>SUB_POW;//Adjust the has for the next round
		
		int cas_fail_count=0;//CAS fail count, as described in put_main

		void *node=getNodeRaw(local,pos);
		// printf("node from getNodeRaw: %p , pos=%d, local=%p\n\r", node, pos, local);
		do{
			if(cas_fail_count>=MAX_CAS_FAILURE){
					#ifdef DEBUGPRINTS_MARK
					printf("Marked a node sub\n");
					#endif
				mark_data_node(local,pos);
			}
			
			//Gets the siginifcant node
			
			
			if(node==NULL){//Same logic as Put Main
			//printf("putIfAbsent_sub if(node==NULL){!\n\r");
				void *node2;
				if( (node2=replace_node(local, pos, NULL, temp_bucket))==node){
					increment_size();//Increments size because an element was insert
					return true;
				}
				else{//CAS Failed
				//	void *node2=getNodeRaw(local,pos);
					if(isSpine(node2)){//If it is a Spine then continue, we don't know if the key was updated. 
						local=unmark_spine(node2);
						break;
					}
					else if(isMarkedData(node2)){
						//Local must be a spine node, so examine the next depth
							#ifdef DEBUGPRINTS_MARK
							printf("Failed as a result of mark sub\n");
							#endif
						node2=forceExpandTable(T,local,pos,unmark_data(node2), right+SUB_POW);
						local=unmark_spine(node2);
						break;
					}
					else if(((DataNode *)node2)->hash==temp_bucket->hash ){//HASH COMPARE
						//See Logic above on why
						NodeDesc* oldCurrDesc = ((DataNode *)node2)->nodeDesc;

						FinishPendingTxn(oldCurrDesc, desc, T);

			            if(IsSameOperation(oldCurrDesc, nodeDesc))
			            {
			                return true;
			            }

			            //if( IsKeyExist( oldCurrDesc) )
						if( IsKeyExist_mod( oldCurrDesc, desc) )
			            	return false;
			            else // the key they wanted to insert, isn't logically in the table so we can insert
		            	{
		            		
		            		NodeDesc* currDesc = ((DataNode *)node2)->nodeDesc;

			                if(desc->status != MAP_ACTIVE)
			                {
			                    return false;
			                }

			                //if(currDesc == oldCurrDesc)
			                {
								//printf("putIfAbsent_sub came here!\n\r");
			                    //Update desc to logically add the key to the table since it's already physically there
			                    currDesc = __sync_val_compare_and_swap(&((DataNode *)node2)->nodeDesc, oldCurrDesc, nodeDesc);

			                    // If the CAS is successful, then the value before the CAS must have been oldCurrDesc which is returned
			                    // to currDesc leading to a successful comparison in the if statement below
			                    if(currDesc == oldCurrDesc)
			                    {
			                        ASSERT_CODE
			                            (
			                             __sync_fetch_and_add(&g_count_ins, 1);
			                            );

			                        // if the logically removed node has a different value, then CAS in our new value
			                        if ( ((DataNode *)node2)->value != temp_bucket->value )
			                        	//if(__sync_bool_compare_and_swap( &((DataNode *)node2)->value, ((DataNode *)node2)->value, ((DataNode* )temp_bucket))->value )
			                        	if(__sync_bool_compare_and_swap( &((DataNode *)node2)->value, ((DataNode *)node2)->value, ((DataNode* )temp_bucket)->value ) )
			                        		return true;
			                        	else // cas failed so retry
			                        		goto insert_sub;
			                        else // had the same value so all we had to do was update the descriptor and return
			                        	return true;
			                    }
			                    else
			                    	goto insert_sub;
			                }
		            	}

			            	//goto noMatch_putSub;
					}
					else{
					//noMatch_putSub:
						cas_fail_count++;
						node=getNodeRaw(local,pos);
						continue;
					}
				}
			}
			else if(isSpine(node)){//If it is a spine break out of the while loop and continue
				//the for loop, then examine the enxt depth
				//printf("putIfAbsent_sub else if(isSpine(node)!\n\r");
				local=unmark_spine(node);
				break;
			}
			else if(isMarkedData(node)){
				//printf("putIfAbsent_sub isMarkedData(node)!\n\r");
					#ifdef DEBUGPRINTS_MARK
					printf("Found marked sub\n");
					#endif
				//Local must be a spine node, so examine the next depth
				node=forceExpandTable(T,local,pos,unmark_data(node), right+SUB_POW);
				local=unmark_spine(node);
				break;
			}
			else{//is Data Node
			//printf("putIfAbsent_sub else!\n\r");
				#ifdef DEBUG
				if( ((DataNode *)node)->hash==0){
					quick_print(local);
					printf("Zero Hash!");
					
				}
				#endif
				if( ((DataNode *)node)->hash==temp_bucket->hash  ){//It is a key match
				//printf("putIfAbsent_sub if( ((DataNode *)node)->hash==temp_bucket->hash  ){//It is a key match!\n\r");
					NodeDesc* oldCurrDesc = ((DataNode *)node)->nodeDesc;
					FinishPendingTxn(oldCurrDesc, desc, T);
					//printf("putIfAbsent_sub after FinishPendingTxn\n\r");

		            if(IsSameOperation(oldCurrDesc, nodeDesc))
		            {
		                return true;
		            }

		            //if( IsKeyExist( oldCurrDesc ) )
					if( IsKeyExist_mod( oldCurrDesc, desc ) )
		            	return false;
	                else // the key they wanted to insert, isn't logically in the table so we can insert
	            	{
						//printf("putIfAbsent_sub  !IsKeyExist\n\r");
	            		
	            		NodeDesc* currDesc = ((DataNode *)node)->nodeDesc;

		                if(desc->status != MAP_ACTIVE)
		                {
		                    return false;
		                }

		                //if(currDesc == oldCurrDesc)
		                {
		                    //Update desc to logically add the key to the table since it's already physically there
		                    currDesc = __sync_val_compare_and_swap(&((DataNode *)node)->nodeDesc, oldCurrDesc, nodeDesc);

		                    // If the CAS is successful, then the value before the CAS must have been oldCurrDesc which is returned
		                    // to currDesc leading to a successful comparison in the if statement below
		                    if(currDesc == oldCurrDesc)
		                    {
		                        ASSERT_CODE
		                            (
		                             __sync_fetch_and_add(&g_count_ins, 1);
		                            );

		                        // if the logically removed node has a different value, then CAS in our new value
		                        if ( ((DataNode *)node)->value != temp_bucket->value )
		                        	//if(__sync_bool_compare_and_swap( &((DataNode *)node)->value, ((DataNode *)node)->value, ((DataNode* )temp_bucket))->value )
		                        	if(__sync_bool_compare_and_swap( &((DataNode *)node)->value, ((DataNode *)node)->value, ((DataNode* )temp_bucket)->value ) )
		                        		return true;
		                        	else // cas failed so retry
		                        		goto insert_sub;
		                        else // had the same value so all we had to do was update the descriptor and return
		                        	return true;
		                    }
		                    else
		                    	goto insert_sub;
		                }
	            	}
		            //else
		            	//goto noMatch_putSub2;
				}
				else{//Create a Spine
				//noMatch_putSub2:
				//printf("putIfAbsent_sub else{//Create a Spine!\n\r");
					bool res=Allocate_Spine(T, local,pos,(DataNode *)node,temp_bucket, right+SUB_POW);
					if(res){
						increment_size();
						return true;
					}
					else{
						cas_fail_count++;
						node=getNodeRaw(local,pos);
						continue;
					}
				}//End Else Create Spine
			}//End Else, Data Node
		}while(true);//End While Loop
	}//End For Loop

//printf("putIfAbsent_sub returns false!\n\r");
	return false;
}//End Sub Put

//TODO: threadid's passed from main.cc start at 1 per maptester's call to workthread
//inline bool Insert(Desc* desc, uint8_t opid, KEY k, VALUE v, int T)
inline bool Update(Desc* desc, uint8_t opid, KEY k,/*VALUE e_value,*/ VALUE v, int T){//, DataNode*& toreturn){//T is the executing thread's ID
	/*if(e_value==v)
		return true;*/

	NodeDesc* nodeDesc = new(m_nodeDescAllocator->Alloc()) NodeDesc(desc, opid);


	
	HASH hash=HASH_KEY(k);//reorders the bits in the key to more evenly distribute the bits
#ifdef useThreadWatch
	Thread_watch[T]=hash;//Buts the hash in the watchlist
#endif

	//Allocates a bucket, then stores the value, key and hash into it

    // DataNode *temp_bucket=(DataNode *) new(m_nodeAllocator->Alloc()) DataNode();
    DataNode *temp_bucket = Allocate_Node(v,hash,T, nodeDesc);


#ifdef DEBUG
	assert(temp_bucket!=NULL);
#endif

	bool res=putUpdate_main(desc, hash,/*e_value,*/ temp_bucket,T, nodeDesc);//, toreturn);
	// if(!res){
	// 	Free_Node_Stack(temp_bucket, T);
	// }

#ifdef useThreadWatch
	Thread_watch[T]=0;//Removes the hash from the watchlist
#endif


	return res;
}

inline bool putUpdate_main(Desc* desc, HASH hash, /*VALUE e_value,*/ DataNode *temp_bucket, int T, NodeDesc* nodeDesc){//, DataNode*& toreturn){

	//This count bounds the number of times the thread will loop as a result of CAS failure.
	int cas_fail_count=0;
update_main:
	//Determines the position to insert at by examining the "M" right most bits
	int pos=getMAINPOS(hash);//&(MAIN_SIZE-1));
#ifdef DEBUG
	assert(pos >=0 && pos <MAIN_SIZE);//Check to make sure position is valid
#endif

	void *node=getNodeRaw(head,pos);//Gets the pointer value at the position of interest
	
	//Examining Main Spine First
	while(true){

		if(cas_fail_count>=MAX_CAS_FAILURE){//Checks to see if it failed the CAS to many times
			mark_data_node(head,pos);//If it has then it marks that node
#ifdef DEBUGPRINTS_MARK
			printf("Marked a Node--MAin\n");
#endif
		}

		if(node==NULL){//See Logic Above
			return false;
		}
		else if(isSpine(node)){//Check the Sub Spines
			return putUpdate_sub(desc, unmark_spine(node),/*e_value,*/ temp_bucket, T, nodeDesc);//, toreturn);

		}
		else if(isMarkedData(node)){//Force Expand The table because someone could not pass the cas
#ifdef DEBUGPRINTS
		    printf("marked found on main!\n");
#endif
		    node=forceExpandTable(T,head,pos,unmark_data(node), MAIN_POW);
			return putUpdate_sub(desc, unmark_spine(node),/*e_value,*/ temp_bucket, T, nodeDesc);//, toreturn);
		}
		else{//It is a Data Node
#ifdef DEBUG
			if( ((DataNode *)node)->hash==0){
				printf("Zero Hash!");
			}
#endif
			if( ((DataNode *)node)->hash==temp_bucket->hash ){//&& IsKeyExist( ((DataNode *)node)->nodeDesc ) ){//It is a key match
				NodeDesc* oldCurrDesc = ((DataNode *)node)->nodeDesc;
				FinishPendingTxn(oldCurrDesc, desc, T);

	            if(IsSameOperation(oldCurrDesc, nodeDesc))
	            {
	                return true;
	            }

	            //if( IsKeyExist( oldCurrDesc ) )
				if( IsKeyExist_mod( oldCurrDesc, desc ) )
	            {
					// we have a key with matching value to update, so update nodedesc
            		NodeDesc* currDesc = ((DataNode *)node)->nodeDesc;

	                if(desc->status != MAP_ACTIVE)
	                {
	                    return false;
	                }

	                if( IsUnsavedUpdate(currDesc, ((DataNode *)node)->value) )
	                {
	                	((DataNode *)node)->value = currDesc->desc->ops[currDesc->opid].value;
	                }

	                //if(currDesc == oldCurrDesc)
	                {
	                    //Update desc to logically add the key to the table since it's already physically there
	                    currDesc = __sync_val_compare_and_swap(&((DataNode *)node)->nodeDesc, oldCurrDesc, nodeDesc);

	                    // If the CAS is successful, then the value before the CAS must have been oldCurrDesc which is returned
	                    // to currDesc leading to a successful comparison in the if statement below
	                    if(currDesc == oldCurrDesc)
	                    {
	                        ASSERT_CODE
	                            (
	                             __sync_fetch_and_add(&g_count_ins, 1);
	                            );

	                        //toReturn = (DataNode *)node;//node;
	                        return true; 
	                    }
	                    else // weren't able to update the descriptor so retry
	                    {
	                    	goto update_main; // restart, preserving fail count and therefore wait-freedom
	                    	// there must be a concurrent transaction for our descriptor update to have failed
	                    	// update nodeinfo algorithm retries if we fail to update the descriptor
	                    }
	                }
				}
				else // the key they wanted to insert, isn't logically in the table so we can insert
            	{
            		return false; // key isn't in the table, so can't update (follows semantics of journal paper)
            	}
				//else
				//	goto noMatch_updateMain;
			}
			else{//Create a Spine
				//Allocate Spine will return true if it succeded, and false if it failed.
				//See Below for functionality.
			//noMatch_updateMain:
				bool res=Allocate_Spine(T, head,pos,(DataNode *)node,temp_bucket, MAIN_POW);
				if(res){
					increment_size();//Increments Size
					return true;
				}
				else{
					cas_fail_count++;
					node=getNodeRaw(head,pos);
					continue;
				}
			}//End Else Create Spine
		}//End Else, Data Node
	}//End While Loop
}//End Update Main

/**
 See Put_Main for logic Discription, this section is vary similar to put_main, but instead allows multiple level traversal
 If you desire 3 or more different lengths of memory array then copy put_main,
 replace MAIN_POW/MAIN_SIZE with correct values, then change put_main's calls to put subs to the copied function

 **DONT FORGET: to do get/delete as well
 */
inline bool putUpdate_sub(Desc* desc, void* /* volatile  */* local, /*VALUE e_value,*/ DataNode *temp_bucket, int T, NodeDesc* nodeDesc){//, DataNode*& toreturn){
update_sub:
		HASH h=(temp_bucket->hash)>>MAIN_POW;//Shifts the hash to move the siginifcant bits to the right most position
	for(int right=MAIN_POW; right<KEY_SIZE; right+=SUB_POW){
		int pos=h&(SUB_SIZE-1);//Gets the sig bits from the hash
#ifdef DEBUG
		assert(pos >=0 && pos <SUB_SIZE);//Check to make sure pos is valid
#endif
		h=h>>SUB_POW;//Adjust the has for the next round
		void *node=getNodeRaw(local,pos);
		
		int cas_fail_count=0;//CAS fail count, as described in put_main
		do{
			if(cas_fail_count>=MAX_CAS_FAILURE){
#ifdef DEBUGPRINTS_MARK
				printf("Marked a node sub\n");
#endif
				mark_data_node(local,pos);
			}

			//Gets the siginifcant node
			

			if(node==NULL){//Same logic as Put Main
				return false;
			}
			else if(isSpine(node)){//If it is a spine break out of the while loop and continue
				//the for loop, then examine the enxt depth
				local=unmark_spine(node);
				break;
			}
			else if(isMarkedData(node)){
#ifdef DEBUGPRINTS_MARK
				printf("Found marked sub\n");
#endif
				//Local must be a spine node, so examine the next depth
				node=forceExpandTable(T,local,pos,unmark_data(node), right+SUB_POW);
				local=unmark_spine(node);
				break;
			}
			else{//is Data Node
#ifdef DEBUG
				if( ((DataNode *)node)->hash==0){
					quick_print(local);
					printf("Zero Hash!");

				}
#endif
				if( ((DataNode *)node)->hash==temp_bucket->hash  ){//It is a key match
					NodeDesc* oldCurrDesc = ((DataNode *)node)->nodeDesc;
					FinishPendingTxn(oldCurrDesc, desc, T);

		            if(IsSameOperation(oldCurrDesc, nodeDesc))
		            {
		                return true;
		            }

		            //if( IsKeyExist( oldCurrDesc ) )
					if( IsKeyExist_mod( oldCurrDesc, desc ) )
		            {
						/*if( ((DataNode *)node)->value != e_value){
							return false;
						}*/

						NodeDesc* currDesc = ((DataNode *)node)->nodeDesc;

		                if(desc->status != MAP_ACTIVE)
		                {
		                    return false;
		                }

		                if( IsUnsavedUpdate(currDesc, ((DataNode *)node)->value) )
		                {
		                	((DataNode *)node)->value = currDesc->desc->ops[currDesc->opid].value;
		                }

		                //if(currDesc == oldCurrDesc)
		                {
		                    //Update desc to logically add the key to the table since it's already physically there
		                    currDesc = __sync_val_compare_and_swap(&((DataNode *)node)->nodeDesc, oldCurrDesc, nodeDesc);

		                    // If the CAS is successful, then the value before the CAS must have been oldCurrDesc which is returned
		                    // to currDesc leading to a successful comparison in the if statement below
		                    if(currDesc == oldCurrDesc)
		                    {
		                        ASSERT_CODE
		                            (
		                             __sync_fetch_and_add(&g_count_ins, 1);
		                            );

		                        //toReturn = (DataNode *)node;//node;
		                        return true; 
		                    }
		                    else
		                    	goto update_sub;
		                }
					}
					else
					{
						return false; //key not there, can't update
					}
				}
				else{//Create a Spine
				//noMatch_updateSub:
					bool res=Allocate_Spine(T, local,pos,(DataNode *)node,temp_bucket, right+SUB_POW);
					if(res){
						increment_size();
						return true;
					}
					else{
						cas_fail_count++;
						node=getNodeRaw(local,pos);
						continue;
					}
				}//End Else Create Spine
			}//End Else, Data Node
		}while(true);//End While Loop
	}//End For Loop
	// printf("888888888888888888888888888\n");
	// exit(8888);
	//NOTE: i don't think this should happen
}//End update sub

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////Get Functions///////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
/**
The get functions retun VALUE if the key is in the table and NULL if it is not.
They don't modify the table and if a data node is marked they ignore the marking

*/

//NOTE: for the map interface, can't return true or false for the first time, have to return a non-null value or null
	// this has to be caught and interpreted by the helpops function when assigning transaction status back to the ret value
	// for the while loop and needs to be done for map interface in general in the update template pseudocode
	//inline VALUE get_first(KEY k, int T){
inline VALUE Find(Desc* desc, uint8_t opid, KEY k, int T){
		NodeDesc* nodeDesc = new(m_nodeDescAllocator->Alloc()) NodeDesc(desc, opid);

		HASH h=HASH_KEY(k);//Reorders the bits for more even distribution
#ifdef useThreadWatch
		Thread_watch[T]=h;//Adds the hash to the watchlist
#endif
		VALUE v=get_main(desc, h, nodeDesc, T);//Calls the get main function							//TODO: MemCopy?
#ifdef useThreadWatch
		Thread_watch[T]=0;//Removes the hash from the watchlist
#endif
		return v;
	}

inline VALUE get_main(Desc* desc, HASH hash, NodeDesc* nodeDesc, int T) {
	find_main:
		int pos=getMAINPOS(hash);//&(MAIN_SIZE-1));
	#ifdef DEBUG	
		assert(pos >=0 && pos <MAIN_SIZE);
	#endif
		
		void *node= getNode(head,pos);//Gets the pointer value at the sig node, stripping bit mark if it had it

		if (node == NULL)
			return (VALUE)NULL;//Returns NULL because key is not in the table
		else if(isSpine(node))
			return get_sub(desc, hash,unmark_spine(node), nodeDesc, T);//Checks the Sub_Spine
		else{//Is Data Node//Found a Data if it is a key match then it returns the value
			if ( ((DataNode *)node)->hash == hash)									//HASH COMPARE
			{
				NodeDesc* oldCurrDesc = ((DataNode *)node)->nodeDesc;
				FinishPendingTxn(oldCurrDesc, desc, T);

	            if(IsSameOperation(oldCurrDesc, nodeDesc))
	            {
	                return 0; //TODO: need to return some value here, maybe make up a sentinel
	            }

	            // NOTE: because we use a perfect hash function with our wfhm, if the key
	            // doesn't exist here, then it doesn't logically exist anywhere in the table 
	            //if( !IsKeyExist( oldCurrDesc ) )
				if( !IsKeyExist_mod( oldCurrDesc, desc ) )
					return (VALUE)NULL;
	            else // the key they wanted to insert, isn't logically in the table so we can insert
            	{
            		
            		NodeDesc* currDesc = ((DataNode *)node)->nodeDesc;

	                if(desc->status != MAP_ACTIVE)
	                {
	                    return (VALUE)NULL;
	                }

	                if( IsUnsavedUpdate(currDesc, ((DataNode *)node)->value) )
	                {
	                	((DataNode *)node)->value = currDesc->desc->ops[currDesc->opid].value;
	                }

// READ_ONLY_OPT_CODE
// (
//         if(desc->isReadOnly)
//             return ((DataNode *)node)->value;
// ) 					

	                //if(currDesc == oldCurrDesc)
	                {
	                    //Update desc to logically add the key to the table since it's already physically there
	                    currDesc = __sync_val_compare_and_swap(&((DataNode *)node)->nodeDesc, oldCurrDesc, nodeDesc);

	                    // If the CAS is successful, then the value before the CAS must have been oldCurrDesc which is returned
	                    // to currDesc leading to a successful comparison in the if statement below
	                    if(currDesc == oldCurrDesc)
	                    {
	                        ASSERT_CODE
	                            (
	                             __sync_fetch_and_add(&g_count_ins, 1);
	                            );

	                        // if this txn did an update here, then use that value
	                        if (IsLiveUpdate(oldCurrDesc))
	                        {
	                        	nodeDesc->desc->ops[nodeDesc->opid].value = GetValue(oldCurrDesc);//oldCurrDesc->desc->ops[oldCurrDesc->opid].value;	
	                        	return nodeDesc->desc->ops[nodeDesc->opid].value;//nodeDesc->value;
	                        }
	                		else
	                		{
	                			// if it's not a live update then use the value at the node
	                			return ((DataNode *)node)->value;
	                		}
	                    }
	                    else
	                    	goto find_main;
	                }

            	}
				//else
					//goto noMatch_getMain;
			}
			else//otherwise return NULL because there is no key match
			{
			//noMatch_getMain:
				return (VALUE)NULL;
			}
		}//End Is Data Node
	}//End Get Main

inline VALUE get_sub(Desc* desc, HASH hash, void* /* volatile  */* local, NodeDesc* nodeDesc, int T){
	find_sub:
		HASH h=hash>>MAIN_POW; //Adjusts the hash bits
		int pos=h&(SUB_SIZE-1);//determines the position to check
		#ifdef DEBUG
		assert(pos >=0 && pos <SUB_SIZE);
		#endif
		
		//This loop will run until the max depth is reached, the spine at the max depth will be examined by 
		//the code block below
		for(int right=MAIN_POW; right<KEY_SIZE-SUB_POW; right+=SUB_POW){
			h=h>>SUB_POW;//adjusts the hash bits for the next time
			void *node=getNode(local,pos);
			if (node == NULL)//See Logic above
				return (VALUE)NULL;
			else if(!isSpine(node)){
				if (((DataNode *)node)->hash == hash)//HASH COMPARE
				{
					NodeDesc* oldCurrDesc = ((DataNode *)node)->nodeDesc;
					FinishPendingTxn(oldCurrDesc, desc, T);

		            if(IsSameOperation(oldCurrDesc, nodeDesc))
		            {
		                return true; //TODO: use sentinel
		            }

		            //if( !IsKeyExist( oldCurrDesc ) )
					if( !IsKeyExist_mod( oldCurrDesc, desc ) )
						return (VALUE)NULL;
		            else // the key they wanted to insert, isn't logically in the table so we can insert
	            	{
	            		
	            		NodeDesc* currDesc = ((DataNode *)node)->nodeDesc;

		                if(desc->status != MAP_ACTIVE)
		                {
		                    return (VALUE)NULL;
		                }

			           	if( IsUnsavedUpdate(currDesc, ((DataNode *)node)->value) )
		                {
		                	((DataNode *)node)->value = currDesc->desc->ops[currDesc->opid].value;
		                }

// READ_ONLY_OPT_CODE
// (
//         if(desc->isReadOnly)
//             return ((DataNode *)node)->value;
// ) 						

		                //if(currDesc == oldCurrDesc)
		                {
		                	//if(IsAbortedUpdate(oldCurrDesc))
		                	//	return oldCurrDesc->desc->ops[oldCurrDesc->opid].value;

		                    //Update desc to logically add the key to the table since it's already physically there
		                    currDesc = __sync_val_compare_and_swap(&((DataNode *)node)->nodeDesc, oldCurrDesc, nodeDesc);

		                    // If the CAS is successful, then the value before the CAS must have been oldCurrDesc which is returned
		                    // to currDesc leading to a successful comparison in the if statement below
		                    if(currDesc == oldCurrDesc)
		                    {
		                        ASSERT_CODE
		                            (
		                             __sync_fetch_and_add(&g_count_ins, 1);
		                            );
		                        
		                        // if this txn did an update here, then use that value
		                        if (IsLiveUpdate(oldCurrDesc))
		                        {
		                        	nodeDesc->desc->ops[nodeDesc->opid].value = GetValue(oldCurrDesc);//nodeDesc->value = oldCurrDesc->desc->ops[oldCurrDesc->opid].value;	
		                        	return nodeDesc->desc->ops[nodeDesc->opid].value;//nodeDesc->value;
		                        }
		                		else
		                		{
		                			// if it's not a live update then use the value at the node
		                			return ((DataNode *)node)->value;
		                		}
		                    }
		                    else
		                    	goto find_sub;
		                }
	            	}
					//else
						//goto noMatch_getSub;
				}
				else
				{
				//noMatch_getSub:
					return (VALUE)NULL;
				}
			}//End Is Data Node
			local=unmark_spine(node);
			pos=h&(SUB_SIZE-1);
		}//End For loop
		
	find_final:
		//Since this is the final depth we dont need to check the hash, as only hash matches can be placed here
		void *node=getNode(local,pos);
		#ifdef DEBUG
		assert(!isSpine(node));
		#endif
		if (node == NULL)
			return (VALUE)NULL;
		else{/* if (((DataNode *)node)->hash == hash)*/ 							//HASH COMPARE
			#ifdef DEBUG
			assert(((DataNode *)node)->hash == hash);
			#endif

			NodeDesc* oldCurrDesc = ((DataNode *)node)->nodeDesc;
			FinishPendingTxn(oldCurrDesc, desc, T);

            if(IsSameOperation(oldCurrDesc, nodeDesc))
            {
                return true; //TODO: use sentinel
            }

            //if( !IsKeyExist( oldCurrDesc ) )
			if( !IsKeyExist_mod( oldCurrDesc, desc ) )
				return (VALUE)NULL;
            else // the key they wanted to insert, isn't logically in the table so we can insert
        	{
        		
        		NodeDesc* currDesc = ((DataNode *)node)->nodeDesc;

                if(desc->status != MAP_ACTIVE)
                {
                    return (VALUE)NULL;
                }

                if( IsUnsavedUpdate(currDesc, ((DataNode *)node)->value) )
                {
                	((DataNode *)node)->value = currDesc->desc->ops[currDesc->opid].value;
                }

// READ_ONLY_OPT_CODE
// (
//         if(desc->isReadOnly)
//             return ((DataNode *)node)->value;
// ) 				

                //if(currDesc == oldCurrDesc)
                {
                    //Update desc to logically add the key to the table since it's already physically there
                    currDesc = __sync_val_compare_and_swap(&((DataNode *)node)->nodeDesc, oldCurrDesc, nodeDesc);

                    // If the CAS is successful, then the value before the CAS must have been oldCurrDesc which is returned
                    // to currDesc leading to a successful comparison in the if statement below
                    if(currDesc == oldCurrDesc)
                    {
                        ASSERT_CODE
                            (
                             __sync_fetch_and_add(&g_count_ins, 1);
                            );

	                        // if this txn did an update here, then use that value
	                        if (IsLiveUpdate(oldCurrDesc))
	                        {
	                        	nodeDesc->desc->ops[nodeDesc->opid].value = GetValue(oldCurrDesc);//nodeDesc->value = //oldCurrDesc->desc->ops[oldCurrDesc->opid].value;	
	                        	return nodeDesc->desc->ops[nodeDesc->opid].value;//nodeDesc->value;
	                        }
	                		else
	                		{
	                			// if it's not a live update then use the value at the node
	                			return ((DataNode *)node)->value;
	                		}
                    }
                    else
                    	goto find_final; // keep retrying until we're aborted by a concurrent txn, or we succeed
                }
        	}

			//return ((DataNode *)node)->value;
		}
		/*else
			return NULL;*/
	}//end get sub    

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////Remove Functions///////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

/**
The Remove functions will remove an element from the hash table if it has a key/hash match
If it removes the element from the table then it will decrement the size of the table.

It returns true if it removed the element, and false if it didn't

If it failes to remove an element, and the current node is now...
	Data Node that is not a key match OR NULL then another thread removed the element from the table, and we can return false, because another thread already removed it
	Spine: then we must examine that spine
	Marked Data Node that is equal to the node we originally tried to CAS
		Then we force create the spine, then try to remove again
	Data Node that is a key match, then we return true and we linearize it that the key was deleted then immeditely inserted
		
**/
	//inline bool remove_first(KEY k, int T){
	//NOTE: nodeDesc is pass by value
inline bool Delete(Desc* desc, uint8_t opid, KEY k, int T){
	//printf("Delete starts!\n\r");
	    NodeDesc* nodeDesc = new(m_nodeDescAllocator->Alloc()) NodeDesc(desc, opid);
		// NodeDesc* nodeDesc = (NodeDesc*)malloc(sizeof(NodeDesc));nodeDesc->desc = desc;nodeDesc->opid=opid;
		HASH h=HASH_KEY(k);//Reorders the bits for more even distribution
#ifdef useThreadWatch
		Thread_watch[T]=h;//Adds the key to the watchlist
#endif

		bool res=remove_main(desc, h, T, nodeDesc);//Checks For the key and removes it if found
#ifdef useThreadWatch
		Thread_watch[T]=0;//Removes the key from the watch list
#endif
		//printf("Delete finishes!\n\r");
		return res;//Returns the result.
	}
	
	inline bool remove_main(Desc* desc, HASH hash, int T, NodeDesc* nodeDesc) {
	delete_main:
		int pos=getMAINPOS(hash);//&(MAIN_SIZE-1));
		#ifdef DEBUG
		assert(pos >=0 && pos <MAIN_SIZE);//Verify the position is valid
		#endif
		
		void *node= getNodeRaw(head,pos);//Gets a Reference to the sig node
		if (node == NULL)
			return false;//Key is not in the table so return false
		if(isMarkedData(node)){//If it is marked the force expand, and examine the sub spine
			node=forceExpandTable(T,head,pos,unmark_data(node), MAIN_POW);
			return remove_sub(desc, hash,unmark_spine(node), T, nodeDesc);
		} 
		if(isSpine(node))//examine the subspine
			return remove_sub(desc, hash,unmark_spine(node), T, nodeDesc);
		
		//Check if it is a Key Match
		if ( ((DataNode *)node)->hash == hash) {//HASH COMPARE
			NodeDesc* oldCurrDesc = ((DataNode *)node)->nodeDesc;
			FinishPendingTxn(oldCurrDesc, desc, T);

            if(IsSameOperation(oldCurrDesc, nodeDesc))
            {
                return true;
            }

            //if( IsKeyExist( oldCurrDesc ) )
			if( IsKeyExist_mod( oldCurrDesc, desc ) )
            {
        		NodeDesc* currDesc = ((DataNode *)node)->nodeDesc;

                if(desc->status != MAP_ACTIVE)
                {
                    return false;
                }

                //if(currDesc == oldCurrDesc)
                {
                    //Update desc to logically add the key to the table since it's already physically there
                    currDesc = __sync_val_compare_and_swap(&((DataNode *)node)->nodeDesc, oldCurrDesc, nodeDesc);

                    // If the CAS is successful, then the value before the CAS must have been oldCurrDesc which is returned
                    // to currDesc leading to a successful comparison in the if statement below
                    if(currDesc == oldCurrDesc)
                    {
                        ASSERT_CODE
                            (
                             __sync_fetch_and_add(&g_count_ins, 1);
                            );

                        return true; 
                    }
                    else
                    	goto delete_main;
                }
			}
            else // the key they wanted to remove, isn't logically in the table so we can't remove
        	{
        		return false;
        	}
			//else
				//goto noMatch_removeMain;
		}//End is Hash match
		
	//noMatch_removeMain:
		return false;
	}//End Remove Main

inline bool remove_sub(Desc* desc, HASH hash, void* /* volatile  */ * local, int T, NodeDesc* nodeDesc) {
	delete_sub:
		HASH h=hash>>MAIN_POW;//Adjusts the hash
		int pos=h&(SUB_SIZE-1);//Gets the position of the sig node
		#ifdef DEBUG
		assert(pos >=0 && pos <SUB_SIZE);//verifies position is valid
		#endif
		
		
		for(int right=MAIN_POW; right<KEY_SIZE; right+=SUB_POW){//Traverses the sub spines
			h= h>>SUB_POW;//Adjusts the hash
			void *node = getNodeRaw(local,pos);//gets the node
			if (node == NULL)//Key is not in the table so return false
				return false;
			if(isMarkedData(node)){//If it is marked then force expand, then check the subspine
				node=forceExpandTable(T,local,pos,unmark_data(node), right+SUB_POW);//Examine the subspine
			}
			else if(!isSpine(node)){//It is a Data Node
				//If it is a key/hash match
				if (( (DataNode *)node)->hash == hash) {//HASH COMPARE
					NodeDesc* oldCurrDesc = ((DataNode *)node)->nodeDesc;
					FinishPendingTxn(oldCurrDesc, desc, T);

		            if(IsSameOperation(oldCurrDesc, nodeDesc))
		            {
		                return true;
		            }

		            //if( IsKeyExist( oldCurrDesc ) )
					if( IsKeyExist_mod( oldCurrDesc, desc ) )
		            {
	            		NodeDesc* currDesc = ((DataNode *)node)->nodeDesc;

		                if(desc->status != MAP_ACTIVE)
		                {
		                    return false;
		                }

		                //if(currDesc == oldCurrDesc)
		                {
		                    //Update desc to logically add the key to the table since it's already physically there
		                    currDesc = __sync_val_compare_and_swap(&((DataNode *)node)->nodeDesc, oldCurrDesc, nodeDesc);

		                    // If the CAS is successful, then the value before the CAS must have been oldCurrDesc which is returned
		                    // to currDesc leading to a successful comparison in the if statement below
		                    if(currDesc == oldCurrDesc)
		                    {
		                        ASSERT_CODE
		                            (
		                             __sync_fetch_and_add(&g_count_ins, 1);
		                            );

		                        return true;
		                    }
		                    else
		                    	goto delete_sub;
		                }

					}
		            else // the key they wanted to insert, isn't logically in the table so we can insert
	            	{
	            		return false;
	            	}
					//else
						//goto noMatch_removeSub;
				} // end hash compare
				else
				{
				//noMatch_removeSub:
					return false;
				}
			}//End Is Data Node
			
			//Sets the current spine, and gets the new position of interst
			local  = unmark_spine(node);
			pos= h&(SUB_SIZE-1);
		}//End For Loop
		
		return false;
	}//End Remove Sub

	/**
	This function is used to expand the table at a position that is under heavy CAS contention.
	IF a thread fails to pass a CAS on a position in X tries, and the current values after those
	CAS operations don't allow the thread to return, the thread will mark the node.
	
	All threads that want to change this node must first expand the table at that position.
	Marking the node is blind, the node can be NULL, a Spine Node, or a Data Node.
	
	If we mark a spine node by mistake there is no issues because the check isSpine only considers a single bit
	that is a different bit then the is marked data. Further the unmarkspine function clears both bits.
	
	If it is null then an empty spine is created at the position.
	
	If it is data then a spine containing an unmarked node is created at the position
	*/
	void * forceExpandTable(int T,void * /* volatile  */ *local,int pos, void *n, int right){
		
		//Gets a Spine node from the Spine pool or allocates a new one
		//See Allocate_Spine for more details on the Spine Pool
		static int fet = 0;
		// printf("forceExpandTable: %d\n\r", fet++);
		void ** s_head =(void**)getSpine();

		
		//Determines the location that the current node belongs in the spine
		int i_pos=0;
		if(n!=NULL){
			DataNode *D=(DataNode *)n;
			i_pos=(	(	(D->hash)>>right	)	&	(SUB_SIZE-1)	);
		}
		
		//Places the node in the spine
		s_head[i_pos]=n;
		//Attempts to CAS the current node for the spine

		void *marked_n=(void *)(mark_data((void *)n));
		void *cas_res;

		if(	(cas_res=replace_node(local, pos, marked_n, (void *)mark_spine(s_head) ) )!=marked_n	){
			//If it fails, remove the node from the spine, place the spine  in the pool, and return the current node
			//The Current Node must be a spine
			// s_head[i_pos]=NULL;
			// s_head[0]=Thread_spines[T];
			// Thread_spines[T]=s_head;
			#ifdef DEBUG
			assert(isSpine(getNode(local,pos)));
			#endif
			return cas_res;
		}
		else{//If it passes, return the spine that was inserted
			assert(isSpine(getNode(local,pos)));
			#ifdef SPINE_COUNT
	 			__sync_fetch_and_add(&spine_elements, 1);
	  		#endif
			return mark_spine(s_head);;
		}
		
	}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////Memory Management Functions/////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////	
	
	/**
	Allocating a Node for reuse:
	
	If a node is removed from the hashtable, then it will be placed in that threads reuse stack or reuse vector
	Each thread has its own stack and a vector of length N-1. A node is placed in the stack if no other thread
	is operating on that node's hash, and placed in the vector if a thread is operating on the node's hash/key.
	
	The length of the vector for each thread is N, since a thread can only be examining one key at a time.
	This means that if the vector is full, and a thread wants to place an element into the array, 
	then there must be an element in the array that is no longer in use, and that element can be moved to the stack
	
	Nodes in the stack, which is a linked list using the the key as a next pointer can be used without examining 
	what the other threads are doing, while node's in the vector can only be used if no other thread is using the same hash.
	
	If there are no nodes in the stack or vector, then the thread will allocate a new node
	
	**/
		inline DataNode * Allocate_Node(VALUE v, HASH h, int T, NodeDesc* nodeDesc){
	
	    DataNode *new_temp_node=(DataNode *) new(m_nodeAllocator->Alloc()) DataNode();
        new_temp_node->value =v;//Assign the value
	    new_temp_node->hash = h;//Assign the hash
		new_temp_node->nodeDesc = nodeDesc;

	    return new_temp_node;//Return        

	}

/**
	This function  allocates the spine nodes used for expansion
	
	IF two nodes collide more than once, then the collision is fully resolved.
	If the thread fails to CAS the spine for the current node, then the allocated spines are placed in a stack
	to be reused later on.
	
	*/
	inline bool Allocate_Spine(int T, void* /* volatile  */* s, int pos, DataNode *n1/*current node*/, DataNode *n2/*colliding node*/, int right){
		//printf("Allocate_Spine begins!\n\r");
		//Gets the hash value of the node to replace
		
		//Change Watch Value
#ifdef useThreadWatch
		Thread_watch[T]=n1->hash;
#endif
		
		/**
		
		This prevents the case where the value is freed, the reallocated and placed in the same position with a different key that was read.
		The current key is read and placed in the watch list. Then It is checked to make sure the node is still at that position. If it is then it checks to make sure the key hasn't changed.
		If the key has changed, which means between the time it was load then stored into the watch list it was freed then reallocated, then it retries to get the key. Otherwise it breaks.
		**/
		int count=0;
#ifdef useThreadWatch
		for(count=0; count<MAX_CAS_FAILURE; count++){
			if( s[pos]!=n1){
				Thread_watch[T]=n2->hash;
				return false;
			}
			else if(n1->hash == Thread_watch[T] ){
				break;
			}
			else{
				Thread_watch[T]=n1->hash;
				continue;
			}
		}
#endif
		if (count==MAX_CAS_FAILURE){
				#ifdef DEBUGPRINTS_MARK
				printf("Marked a node sub--SPINE EXPANSION \n");
				#endif
			mark_data_node(s,pos);
#ifdef useThreadWatch
			Thread_watch[T]=n2->hash;
#endif
			return false;
		}
		
		
		
		//No Sense in going any further if the node is no longer there
		if(s[pos]!=n1 )
			return false;
		
		//Gets the current hash value
		HASH n1_hash	=((n1->hash)>>right);
		HASH n2_hash	=((n2->hash)>>right);
		
		#ifdef SPINE_COUNT
	 	 int spine_count=1;
	  	#endif
		void**s_head;//Gets a spine by either allocting or from the stack
		static int as1 = 0;
		// printf("Allocate_Spine 1: %d\n\r", as1++);
		s_head=(void**)getSpine();
	
		
		//Get the sig positions
		int  n1_pos	=n1_hash&(SUB_SIZE-1);
		int  n2_pos	=n2_hash&(SUB_SIZE-1);
	  //printf("n1_pos==n2_pos %d == %d, n1_hash=%d, n2_hash=%d\n\r", n1_pos,n2_pos, n1_hash, n2_hash);
	 
		void* /* volatile  */ *s_temp=s_head;
	//	int count=1;
		while(n1_pos==n2_pos){//Loop until they no longer collide
		
	// #ifdef DEBUG
			if(n1_hash==n2_hash){
				// printf("Hash Equal Error:2\n"); 
				// print_key(n1->hash); 
				// print_key(n2->hash);
				// check_table_state_p();
				// assert(false);  
#ifdef useThreadWatch
				Thread_Watch[T]=n2->hash;
#endif
				return false;
			}
			assert(n1_hash!=n2_hash);
	// #endif		

			void *s_temp2;
			static int as2 = 0;
			// printf("Allocate_Spine 2: %d, %d==%d, n1_hash=%d, n2_hash=%d\n\r", as2++, n1_pos, n2_pos, n1_hash, n2_hash);
			s_temp2=getSpine();//Returns marked spine

		 #ifdef SPINE_COUNT
	  		spine_count++;
		 #endif
			//Add the spine to the privous spine
			s_temp[n1_pos]=mark_spine((void */* volatile  */*)s_temp2);
			s_temp=(void */* volatile  */*)s_temp2;

			//Adjust the hash and sig positions
			n1_hash	=n1_hash>>SUB_POW;
			n2_hash	=n2_hash>>SUB_POW;
			n1_pos=n1_hash&(SUB_SIZE-1);
			n2_pos=n2_hash&(SUB_SIZE-1);
		}
		//Mem checks
		#ifdef DEBUG
		assert(s_temp!=NULL);
		assert(s_head!=NULL);
		#endif
		
		
		//Adds the nodes to the end spine
		s_temp[0]=NULL;
		s_temp[n1_pos]=n1;
		s_temp[n2_pos]=n2;
	  
	//attemp to add the spine to the table
		void *car_res;
		//printf("before calling replace_node n1=%p\n\r", n1);
		if( (car_res=replace_node(s, pos, (void *)n1,mark_spine((void */* volatile  */*)s_head) ))==n1){
		  //  __sync_fetch_and_add(&v_capacity, count*SUB_SIZE);
	//		printf("Added Spine %p replacing %p (1)\n",s_head,n1);
	  #ifdef SPINE_COUNT
	 	__sync_fetch_and_add(&spine_elements, spine_count);
	  #endif
			return true;//return true if passes
			
		}
		else{//If failed then add the allocated spines to the spine stack
			n2_hash	=(n2->hash>>right);
			
			
// 			void**s_temp=(void **)s_head;
// 			do{//Loops through, moving the spines to position 0, and removing the  node references
// 				n2_pos	=n2_hash&(SUB_SIZE-1);
// 				if(s_temp[n2_pos]==n2){//Break case
// 					s_temp[n1_pos]=NULL;//Remove node n1
// 					s_temp[n2_pos]=NULL;//remove node n2
// 					break;
// 				}
// 				else{//move spine to position 0
// 					s_temp[0]=(void *)unmark_spine(s_temp[n2_pos]);
// 					if(n2_pos!=0){
// 						s_temp[n2_pos]=NULL;
// 					}
// 					s_temp=(void **)s_temp[0];
// 				}
// 				n2_hash= n2_hash>>SUB_POW;
				
// 			}while(true);
// 			s_temp[0]=Thread_spines[T];
			
// 			//Adjust spine stack
// 			Thread_spines[T]=s_head;
// #ifdef useThreadWatch
// 			Thread_watch[T]=n2->hash;
// #endif
			//printf("Allocate_Spine //End Spine Allocator!\n\r");
			return false;
		}
	}//End Spine Allocator	

	//Allocate spine from memory
	inline void * getSpine(){
		//Could add Code to allocate more than one spine
		//printf("getSpine begins!\n\r");
	    MapSpine *new_temp_spine=(MapSpine *) new(m_mapSpineAllocator->Alloc()) MapSpine();
		#ifdef DEBUG
		assert(new_temp_spine!=NULL);
	//	assert(isEmptyArray(s,SUB_SIZE));
		#endif
		//printf("getSpine finishes!\n\r");
		return new_temp_spine;
	}	

// //////Atomic CAS/Writes////
// //SWAPS NULL OR DATA NODE FOR DATA NODE
	inline void * replace_node(void* /* volatile  */ *s , int pos, void *current_node, DataNode *new_node){
		return replace_node(s,pos, (void *) current_node, (void *)new_node);
	}
//SWAPS A DATA NODE FOR SPINE NODE
	inline void * replace_node(void* /* volatile  */ *s, int pos, DataNode *current_node, void* new_node){
		return replace_node(s,pos, (void *) current_node, (void *)new_node);
	}
// //Replaces two pointers
	inline void * replace_node(void* /* volatile  */ *s, int pos, void *current_node, void *new_node) {
		//printf("replace_node begins %p == %p ?\n\r", current_node, s[pos]);
		if (current_node == s[pos]) {
			//printf("replace_node if (current_node == s[pos]) {\n\r");
			return __sync_val_compare_and_swap(&(s[pos]), current_node, new_node);
		}
		//printf("replace_node returns s[pos]=%p\n\r", s[pos]);
		return  s[pos];
	}

// //DELETE NODE!
	inline bool replace_node(void* /* volatile  */ *s, int pos, void *current_node /*, new node=NULL*/) {
		if (current_node == s[pos]) {
			return __sync_bool_compare_and_swap(&(s[pos]), current_node, NULL);
		}
		return false;
	}    

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////Size Calculating Functions//////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
	
inline void increment_size(){
	 __sync_fetch_and_add(&elements, 1);
}
inline void decrement_size(){
	__sync_fetch_and_add(&elements, -1);
}

int size(){ return elements; }


//Debug Interfaces//
int capacity(){
	#ifdef SPINE_COUNT
 		return (SUB_SIZE*spine_elements + MAIN_SIZE);
  	#else
		return -1;
	#endif
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////Atomic Wrapper Functions////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////	

///////Atomic Read/////
inline void * getNode( void* /* volatile  */ *s, int pos){
	return (void *)( ( (unsigned long)(s[pos])) & ~2);
}
inline void * getNodeRaw( void* /* volatile  */ *s, int pos){
	return (void *)(s[pos]);
}

inline bool IsSameOperation(NodeDesc* nodeDesc1, NodeDesc* nodeDesc2)
{
    return nodeDesc1->desc == nodeDesc2->desc && nodeDesc1->opid == nodeDesc2->opid;
}

inline void FinishPendingTxn(NodeDesc* nodeDesc, Desc* desc, int threadId)
{
    // The node accessed by the operations in same transaction is always active 
    if(nodeDesc->desc == desc)
    {
        return;
    }

    //HelpOps(nodeDesc->desc, nodeDesc->opid + 1, threadId);
	HelpOps_rec(nodeDesc->desc, nodeDesc->opid + 1, threadId); 
}

inline bool IsNodeActive(NodeDesc* nodeDesc)
{
    bool ret = (nodeDesc->desc->status == MAP_COMMITTED);

    PERSIST_CODE
    (
        ret = ret && (nodeDesc->desc->persistStatus == PERSISTED);
    )

    return ret;

}

// checks if the node exists
inline bool IsKeyExist(NodeDesc* nodeDesc)
{
    bool isNodeActive = IsNodeActive(nodeDesc);
    uint8_t opType = nodeDesc->desc->ops[nodeDesc->opid].type;

    // NOTE: if the current descriptor is committed, or aborted (not live/active) and referencing an update, then the key exists
    // aborted update operations should have this method return that the key exists, then the old value
    // from the descriptor should be used
    return  (opType == MAP_FIND) || (isNodeActive && opType == MAP_INSERT) || (!isNodeActive && opType == MAP_DELETE) || (opType == MAP_UPDATE);
    // if the operation performed was an update then the key remains in the hash map regardless of return value
    	//((nodeDesc->desc->status == MAP_COMMITTED || nodeDesc->desc->status == MAP_ABORTED) && opType == MAP_UPDATE);
}

inline bool IsNodeActive_mod(NodeDesc* nodeDesc, Desc* desc_this)
{
    bool ret = (nodeDesc->desc->status == MAP_COMMITTED);

    PERSIST_CODE
    (
        ret = ret && (nodeDesc->desc->persistStatus == PERSISTED);
    )

	//BUG FIX
	ret = ret || (nodeDesc->desc == desc_this);

    return ret;

}

// checks if the node exists
inline bool IsKeyExist_mod(NodeDesc* nodeDesc, Desc* desc_this)
{
    bool isNodeActive = IsNodeActive_mod(nodeDesc, desc_this);
    uint8_t opType = nodeDesc->desc->ops[nodeDesc->opid].type;

    // NOTE: if the current descriptor is committed, or aborted (not live/active) and referencing an update, then the key exists
    // aborted update operations should have this method return that the key exists, then the old value
    // from the descriptor should be used
    return  (opType == MAP_FIND) || (isNodeActive && opType == MAP_INSERT) || (!isNodeActive && opType == MAP_DELETE) || (opType == MAP_UPDATE);
    // if the operation performed was an update then the key remains in the hash map regardless of return value
    	//((nodeDesc->desc->status == MAP_COMMITTED || nodeDesc->desc->status == MAP_ABORTED) && opType == MAP_UPDATE);
}

inline bool IsLiveUpdate(NodeDesc* nodeDesc)
{
	if (nodeDesc->desc->ops[nodeDesc->opid].type == MAP_UPDATE || 
	(nodeDesc->desc->ops[nodeDesc->opid].type == MAP_FIND && nodeDesc->desc->ops[nodeDesc->opid].value != 0) )//nodeDesc->value != 0) )
		return true;
	return false;
}

//TODO: this should call IsLiveUpdate
inline bool IsUnsavedUpdate(NodeDesc* nodeDesc, VALUE val)
{
	if (nodeDesc->desc->ops[nodeDesc->opid].value != val && nodeDesc->desc->status == MAP_COMMITTED)
		if (nodeDesc->desc->ops[nodeDesc->opid].type == MAP_UPDATE || 
		(nodeDesc->desc->ops[nodeDesc->opid].type == MAP_FIND && nodeDesc->desc->ops[nodeDesc->opid].value != 0) )
			return true; // else error?
	return false;
}

private:
    // Node* m_tail;
    // Node* m_head;

    void HelpOps(Desc* desc, uint8_t opid, int threadId);
	void HelpOps_rec(Desc* desc, uint8_t opid, int threadId); //CORRECTNESS ANNOTATIONS
	void HelpOpsDBBenchmark(Desc* desc, uint8_t opid, int threadId);
	void HelpOpsTATPBenchmark(Desc* desc, uint8_t opid, int T);//, std::vector<VALUE> &toR)

PERSIST_CODE
(
	std::atomic<uint32_t> lastTxId;

	bool needPersistenceHelp(Desc* desc);
	void persistDesc(Desc* desc);
)	

	Allocator<void **>* m_headAllocator;
    Allocator<DataNode>* m_nodeAllocator;
    Allocator<Desc>* m_descAllocator;
    Allocator<NodeDesc>* m_nodeDescAllocator;
	Allocator<MapSpine>* m_mapSpineAllocator;

    // TODO: make sure counting is disabled during certain performance tests
    ASSERT_CODE
    (
    	// Total node count
        uint32_t g_count = 0;

        uint32_t g_count_ins = 0;
        uint32_t g_count_ins_new = 0;
        uint32_t g_count_del = 0;
        
        // never incremented
        uint32_t g_count_del_new = 0;
        
        uint32_t g_count_fnd = 0;
        uint32_t g_count_upd = 0;
    )

    uint32_t g_count_commit = 0;
    uint32_t g_count_abort = 0;
    uint32_t g_count_fake_abort = 0;


};// end class TransMap


 

#endif /* end of include guard: TRANSMAP_H */
