#ifndef _PERSISTENT_MDLIST_SET_H_
#define _PERSISTENT_MDLIST_SET_H_

#include <string>


/**
 * <h1> A Multi-Dimensional List Set meant to be used with PTMs </h1>
 *
 * see usage example in "main-md.cpp"
 *
 * @created 2019-12-30
 * @author Alexander Goponenko
 *
 *
 */
template<typename K, typename TM, template<typename> class TMTYPE>
class OFMDListSet {

public:
  // NOTE: KeyToCoord()/coord() are implemented only for DIMENSION = 16
  static const uint32_t DIMENSION = 16;

private:
  struct Node
  {
    alignas(128) TMTYPE<Node *> m_child[DIMENSION];
//    alignas(128) TMTYPE<size_t>  m_coord[DIMENSION];
    alignas(128) TMTYPE<K> m_key;

    Node(K key=0):m_key(key) {
      for(uint32_t i=0; i<DIMENSION; i++) {
        m_child[i] = nullptr;
      }
    };

    void deleteDescendants(uint32_t D) {
      //TODO: use D
      for(uint32_t d=0; d < DIMENSION; d++) {
        Node *node = m_child[d];
        if (node) {
          node->deleteDescendants(d);
          TM::tmDelete(node);
        }
      }
    }
  };


private:
  alignas(128) TMTYPE<Node *> m_head{nullptr};

  //TODO: check types
  static uint32_t coord(K key, uint32_t d) {
    static const uint32_t MASK[16] = {0x3u << 30, 0x3u << 28, 0x3u << 26, 0x3u << 24, 0x3u << 22, 0x3u << 20, 0x3u << 18, 0x3u << 16,
                                      0x3u << 14, 0x3u << 12, 0x3u << 10, 0x3u << 8, 0x3u << 6, 0x3u << 4, 0x3u << 2, 0x3u};

    return ((key & MASK[d]) >> (30 - (d << 1)));
  };

  inline void LocatePred(K key, Node*& pred, Node*& cur, uint32_t& cur_dim, uint32_t& pred_dim) {
    uint32_t key_coord; // coordinate of "key" at current dimension
    uint32_t cur_coord; // coordinate of successor at current dimension

    //traverse list from low dim to high dim
    while(cur_dim < DIMENSION)
    {
      key_coord = coord(key, cur_dim);
      // Locate predecessor and successor.
      while(cur && key_coord > (cur_coord = coord(cur->m_key, cur_dim)))
      {
        pred_dim = cur_dim;
        pred = cur;
        cur = cur->m_child[cur_dim];
      }
      // The position of "key" is  after pred. But is it before "curr"?
      if(cur == NULL || key_coord < cur_coord) {
        // Done searching.
        return;
      }
      else
      {
        // continue to search in the next dimension
        // dim only increases if two coords are exactly the same
        ++cur_dim;
      }
    }
  }

public:
  OFMDListSet() {
    TM::template updateTx<bool>([=]() {
        Node *head = TM::template tmNew<Node>();
        m_head = head;
        return true; // Needed for CX
    });
  }

  ~OFMDListSet() {
    TM::template updateTx<bool>([=]() {
        // Delete all the nodes in the list
        Node *head = m_head;
        head->deleteDescendants(0);
        TM::tmDelete(head);
        return true; // Needed for CX
    });
  }

  static std::string className() { return TM::className() + "-MDListSet"; }

  /*
   * Adds a node with a key, returns false if the key is already in the set
   */
  bool add(K key) {
    return TM::template updateTx<bool>([=]() {
        if (key == 0) {
          // key 0 is use for head, thus not allowed
          return false;
        }
        K k = key;
        Node* pred = NULL;      //pred node
        Node* curr = m_head;    //curr node
        uint32_t dim = 0;       //the dimension of curr node
        uint32_t pred_dim = 0;
        LocatePred(k, pred, curr, dim, pred_dim);
        if (dim == DIMENSION) {
          // node with such key already exists
          return false;
        }
        Node* new_node = TM::template tmNew<Node>(k);
        pred->m_child[pred_dim] =  new_node;
        new_node->m_child[dim] = curr;
        for (uint32_t d=pred_dim; d<dim; d++) {
          new_node->m_child[d] = curr->m_child[d];
          curr->m_child[d] = nullptr;
        }
        return true;
    });
  }

  /*
   * Removes a node with an key, returns false if the key is not in the set
   */
  bool remove(K key) {
    return TM::template updateTx<bool>([=]() {
        K k = key;
        Node* pred = NULL;      //pred node
        Node* curr = m_head;    //curr node
        uint32_t dim = 0;       //the dimension of curr node
        uint32_t pred_dim = 0;
        LocatePred(k, pred, curr, dim, pred_dim);
        if (dim != DIMENSION) {
          // node with such key doesn't exists
          return false;
        }
        Node* next;
        for(uint32_t d=DIMENSION-1; d>=pred_dim; d--) {
          next = curr->m_child[d];
          if (next) {
            // smallest non-null child
            for(uint32_t i=pred_dim; i<d; i++) {
              next->m_child[i] = curr->m_child[i];
            }
            break;
          }
        }
        pred->m_child[pred_dim] = next;
        TM::tmDelete(curr);
        return true;
    });
  }

  /*
   * Returns true if it finds a node with a matching key
   */
  bool contains(K key) {
    return TM::template readTx<bool>([=]() {
        K k = key;
        Node* pred = NULL;      //pred node
        Node* curr = m_head;    //curr node
        uint32_t dim = 0;       //the dimension of curr node
        uint32_t pred_dim = 0;
        LocatePred(k, pred, curr, dim, pred_dim);
        return dim == DIMENSION;
    });
  }


  // Used only for benchmarks
  bool addAll(K **keys, const int size) {
    for (int i = 0; i < size; i++) add(*keys[i]);
    return true;
  }
};

#endif /* _PERSISTENT_MDLIST_SET_H_ */
