31,33d30
< bool VERBOSE;
< unsigned int LINEARIZABILITY;
< unsigned int STRICT_SERIALIZABILITY;
522,523c519
< if(VERBOSE) {
< 			if(method_count%5000 == 0 && method_count != 0)
---
> 			if(method_count%5000 == 0)
525d520
< }
859c854
< if(VERBOSE) {
---
> 	#if DEBUG_
861,865c856,857
< 				if(LINEARIZABILITY)
< 					printf("WARNING: Non-Linearizable Read, Item %d, sum_r %.2lf\n", vec_verify->key, vec_verify->sum_r);
< 				else if(STRICT_SERIALIZABILITY)
< 					printf("WARNING: Non-Serializable Read, Item %d, sum_r %.2lf\n", vec_verify->key, vec_verify->sum_r);
< }
---
> 					printf("WARNING: Item %d, sum_r %.2lf\n", vec_verify->key, vec_verify->sum_r);
> 	#endif
877c869
< if(VERBOSE) {
---
> 	#if DEBUG_
879,883c871,872
< 				if(LINEARIZABILITY)
< 					printf("WARNING: Non-Linearizable Failed Method Call, Item %d, sum_f %.2lf\n", vec_verify->key, vec_verify->sum_f);
< 				else if(STRICT_SERIALIZABILITY)
< 					printf("WARNING: Non-Serializable Failed Method Call, Item %d, sum_f %.2lf\n", vec_verify->key, vec_verify->sum_f);
< }
---
> 					printf("WARNING: Item %d, sum_f %.2lf\n", vec_verify->key, vec_verify->sum_f);
> 	#endif
893,898c882,884
< if(VERBOSE) {
< 		if(LINEARIZABILITY)
< 			printf("-------------Execution Linearizable Up To This Time Step------\n");
< 		else if(STRICT_SERIALIZABILITY)
< 			printf("-------------Execution Serializable Up To This Time Step------\n");
< }
---
> #if DEBUG_
> 			printf("-------------Program Correct Up To This Point-------------\n");
> #endif
902,907c888,890
< if(VERBOSE) {
< 		if(LINEARIZABILITY)
< 			printf("-------------Execution Not Linearizable Up To This Time Step--\n");
< 		else if(STRICT_SERIALIZABILITY)
< 			printf("-------------Execution Not Serializable Up To This Time Step--\n");
< }
---
> #if DEBUG_
> 			printf("-------------Program Not Correct-------------\n");
> #endif
917d899
< 	std::stack<int> incorrect_items;
922,926c904,905
< if(VERBOSE) {
< 			printf("Items[%d]->sum = %.2lf\n", i, vector_items1[i]->sum);
< }
< 			if(vector_items1[i]->sum != 0) 
< 			{
---
> 			//printf("vector_items1[%d]->sum = %.2lf\n", i, vector_items1[i]->sum);
> 			if(vector_items1[i]->sum != 0)
928,929d906
< 				incorrect_items.push(i);
< 			}
932,934c909
< if(VERBOSE) {
< 			printf("Items_Durable[%d]->sum = %.2lf\n", i, vector_items2[i]->sum);
< }
---
> 			//printf("vector_items2[%d]->sum = %.2lf\n", i, vector_items2[i]->sum);
936,939c911
< 			{
< 				*outcome_compare = false;
< 				incorrect_items.push(i);	
< 			}
---
> 				*outcome_compare = false;	
942,944c914
< if(VERBOSE) {
< 			printf("Items[%d]->sum = %.2lf, Items_Durable[%d]->sum = %.2lf\n", i, vector_items1[i]->sum, i, vector_items2[i]->sum);
< }
---
> 			//printf("vector_items1[%d]->sum = %.2lf, vector_items2[%d]->sum = %.2lf\n", i, vector_items1[i]->sum, i, vector_items2[i]->sum);
946d915
< 			{
948,953d916
< 				incorrect_items.push(i);
< 			}
< 		} else if(vector_items1[i] == NULL && vector_items2[i] == NULL) {
< if(VERBOSE) {
< 			printf("Items[%d]->sum = 0.00, Items_Durable[%d]->sum = 0.00\n", i, i);
< }
956,964d918
< if(VERBOSE) {
< 	while(!incorrect_items.empty())
< 	{
< 		int top = incorrect_items.top();
< 			
< 		printf("WARNING: NVM not consistent with cache-side Item %d\n", top);
< 		incorrect_items.pop();
< 	}
< }
1422c1376
< 	//printf("VSV_Init()\n");
---
> 	printf("VSV_Init()\n");
1459c1413
< 	//printf("VSV_Init() Finished\n");
---
> 	printf("VSV_Init() Finished\n");
1476,1479c1430
< 		if(LINEARIZABILITY)
< 			printf("-------------Execution Linearizable---------------------------\n");
< 		else if(STRICT_SERIALIZABILITY)
< 			printf("-------------Execution Serializable---------------------------\n");
---
> 		printf("-------------Program Correct Up To This Point-------------\n");
1481,1484c1432
< 		if(LINEARIZABILITY)
< 			printf("-------------Execution Not Linearizable-----------------------\n");
< 		else if(STRICT_SERIALIZABILITY)
< 			printf("-------------Execution Not Serializable-----------------------\n");
---
> 		printf("-------------Program Not Correct-------------\n");
1489,1492c1437
< 		if(LINEARIZABILITY)
< 			printf("-------------Persisted Execution Linearizable-----------------\n");
< 		else if(STRICT_SERIALIZABILITY)
< 			printf("-------------Persisted Execution Serializable-----------------\n");
---
> 		printf("-------------Persisted Program Correct Up To This Point-------------\n");
1494,1497c1439
< 		if(LINEARIZABILITY)
< 			printf("-------------Persisted Execution Not Linearizable-------------\n");
< 		else if(STRICT_SERIALIZABILITY)
< 			printf("-------------Persisted Execution Not Serializable-------------\n");
---
> 		printf("-------------Persisted Program Not Correct-------------\n");
1502,1505c1444
< 		if(final_outcome && final_outcome_persist)
< 			printf("VECTOR SUMS EQUAL => Execution Durable Linearizable\n");
< 		else
< 			printf("VECTOR SUMS EQUAL\n");
---
> 		printf("VECTOR SUMS EQUAL\n");
1507c1446
< 		printf("VECTOR SUMS NOT EQUAL => Execution Not Durable Linearizable\n");
---
> 		printf("VECTOR SUMS NOT EQUAL\n");
1515c1454
< 	printf("Verification Time: %.15lf seconds\n", elapsed_time_double);
---
> 	printf("Total Time: %.15lf seconds\n", elapsed_time_double);
1525c1464
< 	//printf("Total Method Time: %.15lf seconds\n", elapsed_time_method_double);
---
> 	printf("Total Method Time: %.15lf seconds\n", elapsed_time_method_double);
1530c1469
< 	//printf("Total Verification Time: %.15lf seconds\n", elapsed_time_verify_double);
---
> 	printf("Total Verification Time: %.15lf seconds\n", elapsed_time_verify_double);
1618a1558,1578
> 	} else {
> 		
> 		if((got->second.item_key == _item_key) && (got->second.type == PRODUCER && _type == CONSUMER))
> 		{
> 			//Producer followed by Consumer eliminate each other
> 			persist_map[_process].erase(got);
> 		} else if ((got->second.item_key == _item_key) && (got->second.type == CONSUMER && _type == PRODUCER))
> 		{
> 			//Item exists in the persisted state
> 			persist_map[_process].erase(got);
> 			Method m1_persist(-1, _process, _item_key, _item_val, _semantics, _type, 0, 0, true);
> 			std::pair<void*,Method> pair (ptr,m1_persist);
> 			persist_map[_process].insert(pair);
> 		}
> 		/*else if((got->second.item_key == _item_key) && (got->second.type == PRODUCER && _type == WRITER)) {
> 			//Writer overwrite Producer
> 			persist_map[_process].erase(got);
> 			Method m1_persist(-1, _process, _item_key, _item_val, _semantics, _type, 0, 0, true);
> 			std::pair<void*,Method> pair (ptr,m1_persist);
> 			persist_map[_process].insert(pair);
> 		}*/
1636a1597,1598
> 	} else {
> 		//printf("Thread %d: handle_PWB did not find Node %p\n", _process, ptr);
1640d1601
< //TODO: FIXE ME!!!! Major Bug here, method will be inserted in thrd_lists twice when insert_txn is called
1641a1603,1604
> //TODO: FIXE ME!!!! Major Bug here, method will be inserted in thrd_lists twice when insert_txn is called
> #if STRICT_SERIALIZABILITY
1880a1844,1845
> #endif
> 
1933,1938d1897
< 	VERBOSE = false;
< 	LINEARIZABILITY = 1;
< 	STRICT_SERIALIZABILITY = 0;
< 
< 	char vflag[] = "-v";
< 
1947,1961d1905
< 	if(argc > 8) {
< 		if(strcmp (vflag,argv[8]) == 0)
< 		{
< 			VERBOSE = true;
< 			//printf("VERBOSE flag toggled!\n");
< 		} else {
< 			//printf("vflag = %s, argv[8] = %s\n", vflag, argv[8]);
< 		}
< 		
< 	}
< 
< 	if(TRANS_SIZE > 1) {
< 		LINEARIZABILITY = 0;
< 		STRICT_SERIALIZABILITY = 1;
< 	}
1963c1907
< 	//printf("NUM_THRDS = %d, TEST_SIZE = %d, TRANS_SIZE = %d, KEY_RANGE_ = %d\n", NUM_THRDS, TEST_SIZE, TRANS_SIZE, KEY_RANGE_);
---
> 	printf("NUM_THRDS = %d, TEST_SIZE = %d, TRANS_SIZE = %d, KEY_RANGE_ = %d\n", NUM_THRDS, TEST_SIZE, TRANS_SIZE, KEY_RANGE_);
