/*
 * Copyright 2017-2018
 *   Andreia Correia <andreia.veiga@unine.ch>
 *   Pascal Felber <pascal.felber@unine.ch>
 *   Pedro Ramalhete <pramalhe@gmail.com>
 *
 * This work is published under the MIT license. See LICENSE.TXT
 */
#ifndef _TM_LINKED_LIST_SET_H_
#define _TM_LINKED_LIST_SET_H_

#include <string>

#include "../common/tm.h"               // This header defines the macros for the STM being compiled

/**
 * <h1> A Linked List Set for usage with STMs </h1>
 *
 * TODO
 *
 *
 */
template<typename K>
class TMLinkedListSet {

private:

    struct Node {
        TM_TYPE<K>    key;
        TM_TYPE<Node*> next{nullptr};
        Node(const K& key) : key{key}, next{nullptr} { }
        Node(){ }
    };

    alignas(128) TM_TYPE<Node*>  head {nullptr};
    alignas(128) TM_TYPE<Node*>  tail {nullptr};


public:
    TMLinkedListSet() {
		Node* lhead = TM_ALLOC<Node>();
		Node* ltail = TM_ALLOC<Node>();
		head = lhead;
		head->next = ltail;
		tail = ltail;

    }


    ~TMLinkedListSet() {
		// Delete all the nodes in the list
		Node* prev = head;
		Node* node = prev->next;
		while (node != tail) {
			TM_FREE(prev);
			prev = node;
			node = node->next;
		}
		TM_FREE(prev);
		TM_FREE(tail.pload());
    }


    static std::string className() { return TM_NAME() + "-LinkedListSet"; }


    /*
     * Adds a node with a key, returns false if the key is already in the set
     */
    bool add(const K& key) {
        bool retval = false;
        TM_WRITE_TRANSACTION([&] () {
            Node *prev, *node;
            find(key, prev, node);
            retval = !(node != tail && key == node->key);
            if (!retval) return;
            Node* newNode = TM_ALLOC<Node>(key);
            prev->next = newNode;
            newNode->next = node;
        });
        return retval;
    }


    /*
     * Removes a node with an key, returns false if the key is not in the set
     */
    bool remove(const K& key) {
        bool retval = false;
        TM_WRITE_TRANSACTION([&] () {
            Node *prev, *node;
            find(key, prev, node);
            retval = (node != tail && key == node->key);
            if (!retval) return;
            prev->next = node->next;
            TM_FREE(node);
        });
        return retval;
    }


    /*
     * Returns true if it finds a node with a matching key
     */
    bool contains(const K& key) {
        bool retval = false;
        TM_READ_TRANSACTION([&] () {
            Node *prev, *node;
            find(key, prev, node);
            retval = (node != tail && key == node->key);
        });
        return retval;
    }

    void find(const K& key, Node*& prev, Node*& node) {
        for (prev = head; (node = prev->next) != tail; prev = node){
            if ( !(node->key < key) ) break;
        }
    }

    // Used only for benchmarks
    bool addAll(K** keys, const int size) {
    	TM_WRITE_TRANSACTION([&] () {
			bool retval = false;
			for (int i = 0; i < size; i++) {
				Node *prev, *node;
				find(*keys[i], prev, node);
				retval = !(node != tail && *keys[i] == node->key);
				if (retval){
					Node* newNode = TM_ALLOC<Node>(*keys[i]);
					prev->next = newNode;
					newNode->next = node;
				}
			}
    	});
        return true;
    }
};

#endif /* _TM_LINKED_LIST_SET_H_ */
