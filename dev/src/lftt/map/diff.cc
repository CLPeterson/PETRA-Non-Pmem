35,36d34
< //#define USE_MEM_POOL
< 
41,52c39
< 	// inline bool	isSpine(void *s);
< 	// inline void * /* volatile  */ * unmark_spine(void *s);
< 	// inline void *  mark_spine(void * /* volatile  */ *s);
< 
< 	// inline void mark_data_node(void * /* volatile  */*s, int pos);
< 	// inline bool isMarkedData(void *p);
< 	// inline void* mark_data(void *s);
< 	// inline void* unmark_data(void *s);
< 
< 
< 
< 	enum OpStatus
---
> enum OpStatus
59c46
< 	    enum PersistStatus
---
>     enum PersistStatus
66,72d52
< 	// enum ReturnCode
<  //    {
<  //        OK = 0,
<  //        SKIP,
<  //        FAIL
<  //    };
< 
92c72
<             //return sizeof(uint8_t) + sizeof(uint8_t) + sizeof(Operator) * size;
---
>             // return sizeof(uint8_t) + sizeof(uint8_t) + sizeof(Operator) * size;
98c78
< 		volatile uint8_t persistStatus;
---
> 		volatile uint8_t persistStatus;		
100c80
<         bool isReadOnly;
---
> 		bool isReadOnly;
104c84
<     
---
> 
128c108
< 	}DataNode;
---
> 	}DataNode;       
167,169d146
< 
<     
< 
178,180c155,157
< 	
< 	void **Thread_pool_stack;
< 	void **Thread_spines;
---
> 
> 	// void **Thread_pool_stack;
> 	// void **Thread_spines;
194,196d170
< 
< 	
< 
222c196
< 			Thread_pool_stack			=(void **)			calloc(Threads,sizeof(void *));
---
> 			// Thread_pool_stack			=(void **)			calloc(Threads,sizeof(void *));
231,232c205
< 			//Thread_spines			=(void **)		calloc(Threads,sizeof(void *));			
< 
---
> 			
244c217
< 	~TransMap()
---
> ~TransMap()
247c220
< 	    
---
> 
251,256d223
< 	}
< 
< 	// TransMap(/*Allocator<Node>* nodeAllocator,*/ Allocator<Desc>* descAllocator, Allocator<NodeDesc>* nodeDescAllocator, uint64_t initalPowerOfTwo, uint64_t numThreads);
< 	// ~TransMap();
< 
<     //Desc* AllocateDesc(uint8_t size);
258,267c225
<     // inline bool Insert(Desc* desc, uint8_t opid, KEY k, VALUE v, int T);
<     // inline bool Update(Desc* desc, uint8_t opid, KEY k,/*VALUE e_value,*/ VALUE v, int T, DataNode*& toReturn);
<     // inline bool Delete(Desc* desc, uint8_t opid, KEY k, int T);
<     // inline VALUE Find(Desc* desc, uint8_t opid, KEY k, int T);
<     //NOTE: add markfordeletion to Find
< 
< 	//bool ExecuteOps(Desc* desc, int threadId);
< 
< 
< 	
---
> 	}        
313c271
< 	bool ExecuteOps(Desc* desc, int threadId);
---
>     bool ExecuteOps(Desc* desc, int threadId);
317,318c275
< 
< 		//////////////////////////////////////////////////////////////////////////////////
---
> 	//////////////////////////////////////////////////////////////////////////////////
367a325
> 
369c327
< 		int res=1;
---
> 		int res = 1;
379a338,339
> 
> 
382a343
> 	// printf("Insert %d begins!\n\r", k);
384,388c345,346
< //	#ifdef USE_MEM_POOL
< //    	NodeDesc* nodeDesc = new(m_nodeDescAllocator->Alloc()) NodeDesc(desc, opid);
< //    #else
<     	NodeDesc* nodeDesc = (NodeDesc*)malloc(sizeof(NodeDesc));nodeDesc->desc = desc;nodeDesc->opid=opid;
< //	#endif
---
>     NodeDesc* nodeDesc = new(m_nodeDescAllocator->Alloc()) NodeDesc(desc, opid);
> 	// printf("Insert %d after NodeDesc alloc!\n\r", k);
396,400c354,357
< 	//#ifdef USE_KEY
< 	//	DataNode *temp_bucket=Allocate_Node(v,k,hash,T, nodeDesc);
< 	//#else
< 		DataNode *temp_bucket=Allocate_Node(v,hash,T, nodeDesc);
< 	//#endif
---
>     // DataNode *temp_bucket=(DataNode *) new(m_nodeAllocator->Alloc()) DataNode();
>     DataNode *temp_bucket = Allocate_Node(v,hash,T, nodeDesc);
> 	// printf("Insert %d after Allocate_Node!\n\r", k);
> 
406,408c363,366
< 	if(!res){
< 		Free_Node_Stack(temp_bucket, T);
< 	}
---
> 	// printf("Insert %d after putIfAbsent_main!\n\r", k);
> 	// if(!res){
> 	// 	Free_Node_Stack(temp_bucket, T);
> 	// }
412a371
> 	//printf("Insert finishes!\n\r");
415a375
> 
416a377
> 	// printf("putIfAbsent_main begins!\n\r");
553c514
< 
---
> 	//printf("putIfAbsent_main returns false!\n\r");
564a526
> 	//printf("putIfAbsent_sub begins!\n\r");
576a539
> 		// printf("node from getNodeRaw: %p , pos=%d, local=%p\n\r", node, pos, local);
588a552
> 			//printf("putIfAbsent_sub if(node==NULL){!\n\r");
634a599
> 								//printf("putIfAbsent_sub came here!\n\r");
673a639
> 				//printf("putIfAbsent_sub else if(isSpine(node)!\n\r");
677a644
> 				//printf("putIfAbsent_sub isMarkedData(node)!\n\r");
686a654
> 			//printf("putIfAbsent_sub else!\n\r");
694a663
> 				//printf("putIfAbsent_sub if( ((DataNode *)node)->hash==temp_bucket->hash  ){//It is a key match!\n\r");
696a666
> 					//printf("putIfAbsent_sub after FinishPendingTxn\n\r");
707a678
> 						//printf("putIfAbsent_sub  !IsKeyExist\n\r");
748a720
> 				//printf("putIfAbsent_sub else{//Create a Spine!\n\r");
764c736
< 
---
> //printf("putIfAbsent_sub returns false!\n\r");
773,777c745,747
< //	#ifdef USE_MEM_POOL
< //		NodeDesc* nodeDesc = new(m_nodeDescAllocator->Alloc()) NodeDesc(desc, opid);
< //	#else
< 		NodeDesc* nodeDesc = (NodeDesc*)malloc(sizeof(NodeDesc));nodeDesc->desc = desc;nodeDesc->opid=opid;
< //	#endif
---
> 
> 	NodeDesc* nodeDesc = new(m_nodeDescAllocator->Alloc()) NodeDesc(desc, opid);
> 
786,790c756,760
< //#ifdef USE_KEY
< //	DataNode *temp_bucket=Allocate_Node(v,k,hash,T, nodeDesc);
< //#else
< 	DataNode *temp_bucket=Allocate_Node(v,hash,T, nodeDesc);
< //#endif
---
> 
>     // DataNode *temp_bucket=(DataNode *) new(m_nodeAllocator->Alloc()) DataNode();
>     DataNode *temp_bucket = Allocate_Node(v,hash,T, nodeDesc);
> 
> 
796,798c766,768
< 	if(!res){
< 		Free_Node_Stack(temp_bucket, T);
< 	}
---
> 	// if(!res){
> 	// 	Free_Node_Stack(temp_bucket, T);
> 	// }
1073d1042
< //	#ifdef USE_MEM_POOL
1075,1077c1044
< //	#else
< //		NodeDesc* nodeDesc = (NodeDesc*)malloc(sizeof(NodeDesc));nodeDesc->desc = desc;nodeDesc->opid=opid;
< //	#endif
---
> 
1088,1089c1055,1056
< 	
< 	inline VALUE get_main(Desc* desc, HASH hash, NodeDesc* nodeDesc, int T) {
---
> 
> inline VALUE get_main(Desc* desc, HASH hash, NodeDesc* nodeDesc, int T) {
1132a1100,1105
> // READ_ONLY_OPT_CODE
> // (
> //         if(desc->isReadOnly)
> //             return ((DataNode *)node)->value;
> // ) 					
> 
1174,1175c1147,1148
< 	
< 	inline VALUE get_sub(Desc* desc, HASH hash, void* /* volatile  */* local, NodeDesc* nodeDesc, int T){
---
> 
> inline VALUE get_sub(Desc* desc, HASH hash, void* /* volatile  */* local, NodeDesc* nodeDesc, int T){
1218a1192,1197
> // READ_ONLY_OPT_CODE
> // (
> //         if(desc->isReadOnly)
> //             return ((DataNode *)node)->value;
> // ) 						
> 
1303a1283,1288
> // READ_ONLY_OPT_CODE
> // (
> //         if(desc->isReadOnly)
> //             return ((DataNode *)node)->value;
> // ) 				
> 
1339,1340c1324
< 	}//end get sub
< 
---
> 	}//end get sub    
1365,1369c1349,1351
< //	#ifdef USE_MEM_POOL
< 		NodeDesc* nodeDesc = new(m_nodeDescAllocator->Alloc()) NodeDesc(desc, opid);
< //	#else
< //		NodeDesc* nodeDesc = (NodeDesc*)malloc(sizeof(NodeDesc));nodeDesc->desc = desc;nodeDesc->opid=opid;
< //	#endif
---
> 	//printf("Delete starts!\n\r");
> 	    NodeDesc* nodeDesc = new(m_nodeDescAllocator->Alloc()) NodeDesc(desc, opid);
> 		// NodeDesc* nodeDesc = (NodeDesc*)malloc(sizeof(NodeDesc));nodeDesc->desc = desc;nodeDesc->opid=opid;
1378a1361
> 		//printf("Delete finishes!\n\r");
1450,1451c1433,1434
< 	
< 	inline bool remove_sub(Desc* desc, HASH hash, void* /* volatile  */ * local, int T, NodeDesc* nodeDesc) {
---
> 
> inline bool remove_sub(Desc* desc, HASH hash, void* /* volatile  */ * local, int T, NodeDesc* nodeDesc) {
1532c1515

1551,1558c1534,1537
< 		void ** s_head;
< 		if(Thread_spines[T]==NULL){
< 			s_head=(void**)getSpine();
< 		}
< 		else{
< 			s_head=(void**)Thread_spines[T];
< 			Thread_spines[T]=s_head[0];
< 		}
---
> 		static int fet = 0;
> 		// printf("forceExpandTable: %d\n\r", fet++);
> 		void ** s_head =(void**)getSpine();
> 
1577,1579c1556,1558
< 			s_head[i_pos]=NULL;
< 			s_head[0]=Thread_spines[T];
< 			Thread_spines[T]=s_head;
---
> 			// s_head[i_pos]=NULL;
> 			// s_head[0]=Thread_spines[T];
> 			// Thread_spines[T]=s_head;
1618,1620d1596
< 	//#ifdef USE_KEY
< 	//	inline DataNode * Allocate_Node(VALUE v, KEY k, HASH h, int T, NodeDesc* nodeDesc){
< 	//#else
1622d1597
< 	//#endif
1624,1661c1599,1600
< 	    DataNode *new_temp_node=(DataNode *)Thread_pool_stack[T];
< 		
< 	    if(new_temp_node!=NULL){//Check to see if the stack was empty
< 			Thread_pool_stack[T]=new_temp_node->next;//If it wasn't set the stack equal to the next pointer
<             #ifdef DEBUGPRINTS_RECYCLE
< 				printf("Removed From the Stack %p by %d\n",new_temp_node,T);
< 			#endif 
< 			//The Next pointer can be NULL or a data node memory address
< 	    }
< 	    else{//Check the Vector
< #ifdef useVectorPool
< 			int size=Thread_pool_vector_size[T];
< 			for(int i=0; i<size; i++){//Goes through each position in the reuse vector
< 				DataNode *D=(DataNode *)(Thread_pool_vector[T][i]);//Gets the reference
< 				if( (D!=NULL) && (!inUse(D->hash,T)) ){//If it is not null and not inuse, then
< 					Thread_pool_vector[T][i]=NULL;//Set the Position to NULL
< 
< 					#ifdef DEBUGPRINTS_RECYCLE
< 					printf("Removed From the vector %p by %d\n",D,T);
< 					#endif 
< 				
< 					D->value = v;//Assign the value
< 					D->hash = h;//Assign the hash
< 					//#ifdef USE_KEY
< 					//D->key = k;//Assign the key
< 					//#endif
< 					return D;//Return
< 				}//End if
< 			}//End For Loop on Vector
< #endif
< 			//No valid nodes, then malloc
< 			new_temp_node= (DataNode *)calloc(1,sizeof(DataNode));
< 			#ifdef DEBUG
< 			assert(new_temp_node!=NULL);//Insures a node was allocated
< 			#endif
< 	    }//End Else
< 	    
< 	    new_temp_node->value =v;//Assign the value
---
> 	    DataNode *new_temp_node=(DataNode *) new(m_nodeAllocator->Alloc()) DataNode();
>         new_temp_node->value =v;//Assign the value
1663,1666d1601
< 	//#ifdef USE_KEY
< 	//    new_temp_node->key = k;//Assign the key
< 	//#endif
< 	
1669,1671c1604
< 	    return new_temp_node;//Return
< 	}
< 	
---
> 	    return new_temp_node;//Return        
1673,1693d1605
< 	
< 	/*
< 	Adds a node to the stack, used for when a thread fails to insert its node and chooses not to try again.
< 	See logic above put main for scenarios when a threads operation is immeditly replaced
< 	*/
< 	inline void Free_Node_Stack(void *node, int T){
< 		
< 		//Add to Stack
< 		/*if(Thread_pool_stack[T]==NULL){//If null set the next pointer to NULL
< 			Thread_pool_stack[T]=node;
< 			((DataNode *)node)->next=NULL;
< 			return;
< 		}
< 		else{*/
< 			((DataNode *)node)->next=Thread_pool_stack[T];//Sets the next pointer to the stack
< 			Thread_pool_stack[T]=node;//Then set the stack to the node
< 			#ifdef DEBUGPRINTS_RECYCLE
< 			printf("Placed on Stack(4) %p by %d\n",node,T);
< 			#endif
< 			return;
< 	//	}
1695,1698c1607,1608
< 	
< 	
< 	

1706c1616
< inline bool Allocate_Spine(int T, void* /* volatile  */* s, int pos, DataNode *n1/*current node*/, DataNode *n2/*colliding node*/, int right){
---
> 	inline bool Allocate_Spine(int T, void* /* volatile  */* s, int pos, DataNode *n1/*current node*/, DataNode *n2/*colliding node*/, int right){
1866c1776
< 	
---
> 
1870c1780,1781
< 		void* s=(void*)calloc(SUB_SIZE,(sizeof(void */* volatile  */)));
---
> 		//printf("getSpine begins!\n\r");
> 	    MapSpine *new_temp_spine=(MapSpine *) new(m_mapSpineAllocator->Alloc()) MapSpine();
1872c1783
< 		assert(s!=NULL);
---
> 		assert(new_temp_spine!=NULL);
1875,1892c1786,1789
< 		return s;
< 	}
<    
< 	/*Loop through the thread watch list and if any thread is watching the hash value
< 		Then return true, otherwise return false.
< 		We can ignore our own thread
< 	*/
< #ifdef useThreadWatch
< 	inline bool inUse(HASH h, int T){
< 		//Thread_pool_stack		Thread_pool_vector
<         for(int i=0; i<Threads; i++){
<             if(h==Thread_watch[i] && T!=i)//HASH Compare
<                 return true;
<         }
<         return false;
< 	}; //TODO: pretty sure this semicolon is just ignored
< #endif
< 	
---
> 		//printf("getSpine finishes!\n\r");
> 		return new_temp_spine;
> 	}	
> 
1904c1801
< 		
---
> 		//printf("replace_node begins %p == %p ?\n\r", current_node, s[pos]);
1905a1803
> 			//printf("replace_node if (current_node == s[pos]) {\n\r");
1907a1806
> 		//printf("replace_node returns s[pos]=%p\n\r", s[pos]);
1917c1816
< 	}
---
> 	}    
1944,1945d1842
< 
< 
1974c1871
< 	HelpOps_rec(nodeDesc->desc, nodeDesc->opid + 1, threadId);
---
> 	HelpOps_rec(nodeDesc->desc, nodeDesc->opid + 1, threadId); 
1979c1876,1884
<     return nodeDesc->desc->status == MAP_COMMITTED;
---
>     bool ret = (nodeDesc->desc->status == MAP_COMMITTED);
> 
>     PERSIST_CODE
>     (
>         ret = ret && (nodeDesc->desc->persistStatus == PERSISTED);
>     )
> 
>     return ret;
> 
2061d1965
< 
2088c1992,1995
< }; // end class TransMap
---
> 
> 
> };// end class TransMap
> 
2092c1999
< #endif /* end of include guard: TRANSMAP_H */    
---
> #endif /* end of include guard: TRANSMAP_H */
