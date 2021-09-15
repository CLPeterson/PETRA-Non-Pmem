#ifndef _PERSISTENT_ROMULUS_TM_SKIP_LIST_H_
#define _PERSISTENT_ROMULUS_TM_SKIP_LIST_H_

#include <string>
#include "../commonromulus/tm.h" 

#define NUM_LEVELS 20

/**
 * <h1> A Skip List for PTMs </h1>
 */
template<typename K>
class TMSkipList
{

private:
    alignas(128) unsigned int seed;

    struct Node
    {
        TM_TYPE<K> key;
        TM_TYPE<Node *> next[NUM_LEVELS];
        TM_TYPE<int> height;
        Node(int nodeHeight)
        {
            height = nodeHeight;
        }
        Node(const K &lkey, int nodeHeight)
        {
            key = lkey;
            height = nodeHeight;
        }
    };

    alignas(128) TM_TYPE<Node *> head{nullptr};
    alignas(128) TM_TYPE<Node *> tail{nullptr};

    /*
     * Generates a random height between 1 and NUM_LEVELS using the given seed.
     * Notice that currently the datastructure has a single, shared seed which may
     * impact performance in add-heavy benchmarks.
     */
    static int gen_height(unsigned int &seed)
    {
        unsigned long r = rand_r(&seed);
        int l = 1;
        r = (r >> 4) & ((1 << (NUM_LEVELS - 1)) - 1);
        while ((r & 1))
        {
            l++;
            r >>= 1;
        }
        return l;
    }

    /*
     * Returns the first node at the specified level with a key value greater than 
     * or equal to lkey starting at the specified node.
     */
    void find(const K &lkey, Node *start, Node *&prev, Node *&node, int level = 0)
    {
        Node *ltail = tail;
        for (prev = start; (node = (prev->next)[level]) != ltail; prev = node)
        {
            if (node == tail || node == head || node->key >= lkey)
                break;
        }
    }

public:
    TMSkipList(unsigned int rand_seed = -1)
    {
        seed = rand_seed;
        if (rand_seed == -1)
            seed = time(NULL);

        Node *lhead = TM_ALLOC<Node>(NUM_LEVELS);
        Node *ltail = TM_ALLOC<Node>(NUM_LEVELS);

        head = lhead;
        for (int h = 0; h < NUM_LEVELS; h++)
        {
            (head->next)[h] = ltail;
        }
        tail = ltail;

        // return true; // Needed for CX
    }

    ~TMSkipList()
    {
            // Delete all the nodes in the list
            Node *prev = head;
            Node *node = (prev->next)[0];
            while (node != tail)
            {
                TM_FREE(prev);
                prev = node;
                node = (node->next)[0];
            }
            TM_FREE(prev);
            TM_FREE(tail.pload());
            // return true; // Needed for CX
    }

    static std::string className() { return TM_NAME() + "-SkipList"; }

    /*
     * Adds a node with the specified key. If other nodes with the same key exist,
     * the node will be added before those nodes.
     */
    bool add(K key)
    {
        bool retval = false;
// #ifndef PMDK_PTM        
//         TM_WRITE_TRANSACTION([&] () {
// #endif  
            K lkey = key;
            if (contains(lkey))
                return false;            
            Node *prev, *node, *start = head;
            Node *newNode = TM_ALLOC<Node>(lkey, gen_height(seed));

            for (int h = NUM_LEVELS - 1; h > newNode->height - 1; h--)
            {
                find(lkey, start, prev, node, h);
                start = prev;
                // if(node->key == lkey) {
                //     TM_FREE(newNode);
                //     return false;
                // }
                
            }

            for (int h = newNode->height - 1; h >= 0; h--)
            {
                find(lkey, start, prev, node, h);
                // if(node->key == lkey) {
                //     TM_FREE(newNode);
                //     return false;
                // }                
                (newNode->next)[h] = node;
                (prev->next)[h] = newNode;
                start = prev;
            }

            retval = true;
// #ifndef PMDK_PTM
//         });
// #endif   
        return retval;
    }

    /*
     * Removes the node with the key, returns false if the key is not in the set.
     * If there are multiple nodes with a matching key, it will remove the first 
     * in the list.
     */
    bool remove(K key)
    {
        bool retval = false;
// #ifndef PMDK_PTM        
//         TM_WRITE_TRANSACTION([&] () {
// #endif 
            K lkey = key;
            Node *start = head, *prev, *node, *target = nullptr;

            // Find first node matching the key
            for (int h = NUM_LEVELS - 1; h >= 0; h--)
            {
                find(lkey, start, prev, node, h);
                start = prev;
            }

            int k = node->key;
            if (node == tail || k != lkey) {
                retval = false;
// #ifndef PMDK_PTM
//             return;
// #else
            return retval;
// #endif
            }
                // return false;

            target = node;
            start = head;

            // Update pointers for each level of removed node
            for (int h = NUM_LEVELS - 1; h > target->height - 1; h--)
            {
                find(lkey, start, prev, node, h);
                start = prev;
            }

            for (int h = target->height - 1; h >= 0; h--)
            {
                find(lkey, start, prev, node, h);
                start = prev;

                (prev->next)[h] = (node->next)[h];
            }

            TM_FREE(node);
            retval = true;
// #ifndef PMDK_PTM
//         });
// #endif  
        return retval;
    }

    /*
     * Returns true if it finds a node with a matching key
     */
    bool contains(K key)
    {
        bool retval = false;
// #ifndef PMDK_PTM        
//         TM_READ_TRANSACTION([&] () {
// #endif  
            K lkey = key;
            Node *start = head, *prev, *node;

            for (int h = NUM_LEVELS - 1; h >= 0; h--)
            {

                find(lkey, start, prev, node, h);

                start = prev;

                if (node != tail && node->key == lkey) {
                    retval = true;
// #ifndef PMDK_PTM
//             return;
// #else
            return retval;
// #endif
                }
                    // return true;
            }

            retval = false;
// #ifndef PMDK_PTM
//         });
// #endif 
        return retval;
    }

    /*
     * Prints the data structure in a human-readable format
     */
    void debugPrint()
    {
        /*
        return TM::template updateTx<bool>([=]() {
            printf("\n--- Printing Skip List ---\n");
            Node *node;
            Node *ltail = tail;
            for (node = head;; node = (node->next)[0])
            {
                int ky = node->key;
                if (node == head || node == tail)
                    ky = -1;
                printf("Addr: %p, Key: %d, Height: %d, Next Array: {", node, ky, node->height);
                fflush(stdout);
                if (node == ltail)
                {
                    printf("N/A}\n");
                    break;
                }
                for (int i = 0; i < node->height; i++)
                {
                    int ky = -1;
                    if ((node->next)[i] != ltail && (node->next)[i] != head)
                        ky = (node->next)[i]->key;
                    printf("\"Key: %3d\"", ky);
                    if (i != node->height - 1)
                        printf(", ");
                    fflush(stdout);
                }
                printf("}\n");
            }
            printf("--- Print Complete ---\n\n");
            return true;
        });
        */
    }
};

#endif /* _PERSISTENT_TM_SKIP_LIST_H_ */
