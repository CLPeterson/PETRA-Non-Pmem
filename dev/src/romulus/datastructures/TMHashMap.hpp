/*
 * Copyright 2017-2018
 *   Andreia Correia <andreia.veiga@unine.ch>
 *   Pascal Felber <pascal.felber@unine.ch>
 *   Pedro Ramalhete <pramalhe@gmail.com>
 *
 * This work is published under the MIT license. See LICENSE.TXT
 */
#ifndef _TM_RESIZABLE_HASH_MAP_H_
#define _TM_RESIZABLE_HASH_MAP_H_

#include <string>

#include "../commonromulus/tm.h"               // This header defines the macros for the STM being compiled

/**
 * <h1> A Resizable Hash Map for usage with STMs </h1>
 * TODO
 *
 */
template<typename K, typename V>
class TMHashMap {

private:
    struct Node {
        TM_TYPE<K>     key;
        TM_TYPE<V>     val;
        TM_TYPE<Node*> next {nullptr};
        Node(const K& k, const V& v) : key{k}, val{v} { } // Copy constructor for k and value
    };


    TM_TYPE<long>                         capacity;
    TM_TYPE<long>                         sizeHM = 0;
    TM_TYPE<double>                       loadFactor = 0.75;
    alignas(128) TM_TYPE<TM_TYPE<Node*>*> buckets;      // An array of pointers to Nodes


public:
    TMHashMap(int capacity=4) : capacity{capacity} {
		buckets = (TM_TYPE<Node*>*)TM_PMALLOC(capacity*sizeof(TM_TYPE<Node*>));
		for (int i = 0; i < capacity; i++) buckets[i]=nullptr;
    }


    ~TMHashMap() {
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


    std::string className() { return TM_NAME() + "-HashMap"; }


    void rebuild() {
        int newcapacity = 2*capacity;
        TM_TYPE<Node*>* newbuckets = (TM_TYPE<Node*>*)TM_PMALLOC(newcapacity*sizeof(TM_TYPE<Node*>));
        for (int i = 0; i < newcapacity; i++) newbuckets[i] = nullptr;
        for (int i = 0; i < capacity; i++) {
            Node* node = buckets[i];
            while(node!=nullptr){
                Node* next = node->next;
                auto h = std::hash<K>{}(node->key) % newcapacity;
                node->next = newbuckets[h];
                newbuckets[h] = node;
                node = next;
            }
        }
        TM_PFREE(buckets);
        buckets = newbuckets;
        capacity = newcapacity;
    }


    /*
     * Adds a node with a key if the key is not present, otherwise replaces the value.
     * If saveOldValue is set, it will set 'oldValue' to the previous value, iff there was already a mapping.
     *
     * Returns true if there was no mapping for the key, false if there was already a value and it was replaced.
     */
    bool innerPut(const K& key, const V& value, V& oldValue, const bool saveOldValue) {
        if (sizeHM > capacity*loadFactor) rebuild();
        auto h = std::hash<K>{}(key) % capacity;
        Node* node = buckets[h];
        Node* prev = node;
        while (true) {
            if (node == nullptr) {
                Node* newnode = TM_ALLOC<Node>(key,value);
                if (node == prev) {
                    buckets[h] = newnode;
                } else {
                    prev->next = newnode;
                }
                sizeHM++;
                return true;  // New insertion
            }
            if (key == node->key) {
                if (saveOldValue) oldValue = node->val; // Makes a copy of V
                node->val = value;
                return false; // Replace value for existing key
            }
            prev = node;
            node = node->next;
        }
    }


    /*
     * Removes a key and its mapping.
     * Saves the value in 'oldvalue' if 'saveOldValue' is set.
     *
     * Returns returns true if a matching key was found
     */
    bool innerRemove(const K& key, V& oldValue, const bool saveOldValue) {
        auto h = std::hash<K>{}(key) % capacity;
        Node* node = buckets[h];
        Node* prev = node;
        while (true) {
            if (node == nullptr) return false;
            if (key == node->key) {
                if (saveOldValue) oldValue = node->val; // Makes a copy of V
                if (node == prev) {
                    buckets[h] = node->next;
                } else {
                    prev->next = node->next;
                }
                sizeHM--;
                TM_FREE(node);
                return true;
            }
            prev = node;
            node = node->next;
        }
    }


    /*
     * Returns true if key is present. Saves a copy of 'value' in 'oldValue' if 'saveOldValue' is set.
     */
    bool innerGet(const K& key, V& oldValue, const bool saveOldValue) {
        auto h = std::hash<K>{}(key) % capacity;
        Node* node = buckets[h];
        while (true) {
            if (node == nullptr) return false;
            if (key == node->key) {
                if (saveOldValue) oldValue = node->val; // Makes a copy of V
                return true;
            }
            node = node->next;
        }
    }


    //
    // Set methods for running the usual tests and benchmarks
    //

    // Inserts a key only if it's not already present
    bool add(const K& key) {
        bool retval = false;
// #ifndef PMDK_PTM        
//         TM_WRITE_TRANSACTION([&] () {
// #endif
            V notused;
            retval = innerPut(key,key,notused,false);
// #ifndef PMDK_PTM
//         });
// #endif  
        return retval;
    }

    // Returns true only if the key was present
    bool remove(const K& key) {
        bool retval = false;
// #ifndef PMDK_PTM        
//         TM_WRITE_TRANSACTION([&] () {
// #endif
            V notused;
            retval = innerRemove(key,notused,false);
// #ifndef PMDK_PTM
//         });
// #endif  
        return retval;
    }

    bool contains(const K& key) {
        bool retval = false;
// #ifndef PMDK_PTM        
//         TM_READ_TRANSACTION([&] () {
// #endif 
            V notused;
            retval = innerGet(key,notused,false);
// #ifndef PMDK_PTM
//         });
// #endif  
        return retval;
    }

    // Used only for benchmarks
    void addAll(K** keys, const int size) {
    	TM_WRITE_TRANSACTION([&] () {
        	V notused;
            for (int i = 0; i < size; i++) {
            	innerPut(*keys[i],*keys[i],notused,false);
            }
    	});
    }
};

#endif /* _TM_RESIZABLE_HASH_MAP_H_ */
