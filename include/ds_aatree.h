#ifndef DS_AATREE_H
#define DS_AATREE_H

#include "ds_util.h"

template< typename ActualNodeType >
class AATree;

template< typename ActualNodeType >
class AATreeNode {
private:
  ActualNodeType *m_left, *m_right;
  int m_level;

  NO_VALUE_SEMANTICS( AATreeNode );

public:
  AATreeNode() : m_left( nullptr ), m_right( nullptr ), m_level( 1 ) { }
  ~AATreeNode() { }

  ActualNodeType *get_left() const { return m_left; }
  void set_left( ActualNodeType *left ) { m_left = left; }

  ActualNodeType *get_right() const { return m_right; }
  void set_right( ActualNodeType *right ) { m_right = right; }

  int get_level() const { return m_level; }
  void set_level( int level ) { m_level = level; }

  // Get pointer to this node's left pointer.
  // Only AATree should  call this function.
  ActualNodeType **get_ptr_to_left() { return &m_left; }

  // Get pointer to this node's right pointer.
  // Only AATree should  call this function.
  ActualNodeType **get_ptr_to_right() { return &m_right; }
};

template< typename ActualNodeType >
class AATree {
public:
  //! Type of node comparison function: returns true IFF left node
  //! compares as less than right node
  typedef bool LessThanFn( const ActualNodeType *left, const ActualNodeType *right );

  // TODO: we'll likely need a NodeSwapContentsFn function type,
  // because for deletion we'll need to move the contents of a leaf
  // to the position of the node whose contents are being deleted

  //! Node free function type
  typedef void FreeNodeFn( ActualNodeType *node );

private:
  ActualNodeType *m_root;
  LessThanFn *m_less_than_fn;
  FreeNodeFn *m_free_node_fn;

  NO_VALUE_SEMANTICS( AATree );

  // Assume that the height of the tree will never be greater than this:
  // allows for using fixed-size arrays to keep track of nodes along
  // a path from root to leaf
  static constexpr const int MAX_HEIGHT = 32;

public:
  AATree( LessThanFn *less_than_fn, FreeNodeFn *free_node_fn )
    : m_root( nullptr )
    , m_less_than_fn( less_than_fn )
    , m_free_node_fn( free_node_fn ) {
  }

  ~AATree() {
    // TODO: free nodes
  }

  //! Insert given node into the AATree.
  //! @param node the node to insert
  //! @return true if the node is inserted successfully, in which case
  //!         the AATree assumes ownership of it, or false if a node
  //!         comparing as equal already exists in the AATree,
  //!         in which case the node remains the caller's responsibility
  bool insert( ActualNodeType *node ) {
    // The node should be in its initial state
    DS_ASSERT( node->get_left() == nullptr );
    DS_ASSERT( node->get_right() == nullptr );
    DS_ASSERT( node->get_level() == 1 );

    // Keep track of pointers that may need to be updated
    ActualNodeType **path[ MAX_HEIGHT ];
    int path_len = 0;
    ActualNodeType **link = &m_root;

    while ( *link != nullptr ) {
      DS_ASSERT( path_len < MAX_HEIGHT );
      path[ path_len ] = link;
      ++path_len;
      if ( m_less_than_fn( node, *link ) )
        link = (*link)->get_ptr_to_left();
      else {
        if ( !m_less_than_fn( *link, node ) )
          return false; // node compares as equal to an existing node
        link = (*link)->get_ptr_to_right();
      }
    }

    // Attach the node
    *link = node;

    // TODO: rebalance

    return true;
  }

  //! Search for a node in the tree comparing as equal to the given one.
  //! @param node a node
  //! @return pointer to a tree nod equal to the given node,
  //!         or nullptr if the tree does not contain a node equal to
  //!         the given one
  ActualNodeType *find( const ActualNodeType &node ) const {
    ActualNodeType *p = m_root;
    while ( p != nullptr ) {
      if ( m_less_than_fn( &node, p ) )
        p = p->get_left();     // continue in left subtree
      else if ( !m_less_than_fn( p, &node ) )
        return p;              // p is equal to the given node
      else
        p = p->get_right();    // continue in right subtree
    }
    return nullptr;            // search failed
  }

  //! Determine if the tree contains a node equal to the given one.
  //! @param node a node
  //! @return true if the tree contains a node equal to the given one,
  //!         false otherwise
  bool contains( const ActualNodeType &node ) const {
    return find( node ) != nullptr;
  }
};

#endif // DS_AATREE_H
