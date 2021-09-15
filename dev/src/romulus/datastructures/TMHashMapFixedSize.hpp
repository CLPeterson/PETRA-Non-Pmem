/*
 * Copyright 2017-2018
 *   Andreia Correia <andreia.veiga@unine.ch>
 *   Pascal Felber <pascal.felber@unine.ch>
 *   Pedro Ramalhete <pramalhe@gmail.com>
 *
 * This work is published under the MIT license. See LICENSE.TXT
 */
#ifndef _TM_Fixed_HASH_MAP_H_
#define _TM_Fixed_HASH_MAP_H_

#include <string>

#include "../commonromulus/tm.h"               // This header defines the macros for the STM being compiled

/**
 * <h1> A Resizable Hash Map for usage with STMs </h1>
 * TODO
 *
 */
template<typename K, typename V>
class TMHashMapFixedSize {

private:
    struct Node {
        TM_TYPE<K>    key;
        TM_TYPE<V>    val;
        TM_TYPE<Node*> next {nullptr};
        Node(const K& k, const V& value) : key{k}, val{value} { }
    };


    TM_TYPE<long>                         capacity;
    alignas(128) TM_TYPE<TM_TYPE<Node*>*> buckets;      // An array of pointers to Nodes


public:
    // The default size is hard-coded to 2048 entries in the buckets array
    TMHashMapFixedSize(int capacity=2048) : capacity{capacity} {
		buckets = (TM_TYPE<Node*>*)TM_PMALLOC(capacity*sizeof(TM_TYPE<Node*>));
		for (int i = 0; i < capacity; i++) buckets[i]=nullptr;
    }


    ~TMHashMapFixedSize() {
		for(int i = 0; i < capacity; i++){
			Node* node = buckets[i];
			while (node!=nullptr) {
				Node* next = node->next;
				TM_FREE(node);
				node = next;
			}
		}
		TM_PFREE(buckets);
    }


    std::string className() { return TM_NAME() + "-HashMapFixedSize"; }


    /*
     * Adds a node with a key if the key is not present, otherwise replaces the value.
     * Returns the previous value (nullptr by default).
     */
    bool innerPut(const K& key, const V& value, V& oldValue, const bool saveOldValue) {
        auto h = std::hash<K>{}(key) % capacity;
        Node* node = buckets[h];
        Node* prev = node;
        V* oldVal = nullptr;
        while (true) {
            if(node==nullptr){
                Node* newnode = TM_ALLOC<Node>(key,value);
                if(node==prev){
                    buckets[h] = newnode;
                }else{
                    prev->next = newnode;
                }
                return true;
            }
            if(node->key == key){
            	if (saveOldValue) oldValue = node->val; // Makes a copy of V
                node->val = value;
                return false;
            }
            prev = node;
            node = node->next;
        }
    }

    /*
     * Removes a key, returning the value associated with it.
     * Returns nullptr if there is no matching key.
     */
    bool innerRemove(const K& key, V& oldValue, const bool saveOldValue) {
        auto h = std::hash<K>{}(key) % capacity;
        Node* node = buckets[h];
        Node* prev = node;
        while (true) {
            if (node == nullptr) return false;
            if (node->key == key) {
            	if (saveOldValue) oldValue = node->val; // Makes a copy of V
                if(node==prev){
                    buckets[h] = node->next;
                }else{
                    prev->next = node->next;
                }
                TM_FREE(node);
                return true;
            }
            prev = node;
            node = node->next;
        }
    }


    /*
     * Returns the value associated with the key, nullptr if there is no mapping
     */
    bool innerGet(const K& key, V& oldValue, const bool saveOldValue) {
        auto h = std::hash<K>{}(key) % capacity;
        Node* node = buckets[h];
        while (true) {
            if (node == nullptr) return false;
            if (node->key == key) {
                if (saveOldValue) oldValue = node->val; // Makes a copy of V
                return true;
            }
            node = node->next;
        }
    }


    //
    // Set methods for running the usual tests and benchmarks
    //

    bool add(const K& key) {
        bool retval = false;
        TM_WRITE_TRANSACTION([&] () {
            V notused;
            retval = innerPut(key,key,notused,false);
        });
        return retval;
    }

    bool add(const K& key, const V& value) {
        bool retval = false;
        TM_WRITE_TRANSACTION([&] () {
            V notused;
            retval = innerPut(key,value,notused,false);
        });
        return retval;
    }

    bool remove(const K& key) {
        bool retval = false;
        TM_WRITE_TRANSACTION([&] () {
            V notused;
            retval = innerRemove(key,notused,false);
        });
        return retval;
    }

    bool contains(const K& key) {
        bool retval = false;
        TM_READ_TRANSACTION([&] () {
            V notused;
            retval = innerGet(key,notused,false);
        });
        return retval;
    }

    // Used only for benchmarks
    void addAll(K** keys, const int size) {
    	V notused;
    	TM_WRITE_TRANSACTION([&] () {
			for (int i = 0; i < size; i++) {
				innerPut(*keys[i],*keys[i],notused,false);
			}
    	});
    }

    // Used only for benchmarks
    void addAll(K** keys, V value, const int size) {
    	V notused;
    	TM_WRITE_TRANSACTION([&] () {
			for (int i = 0; i < size; i++) {
				innerPut(*keys[i],value,notused,false);
			}
    	});
    }
};

#endif /* _TM_Fixed_HASH_MAP_H_ */
