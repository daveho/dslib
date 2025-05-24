#ifndef DS_AATREE_H
#define DS_AATREE_H

#include "ds_util.h"

namespace dslib {

//! Intrusive AA tree node base class.
class AATreeNode {
private:
  AATreeNode *m_left, *m_right;
  int m_level;

  NO_VALUE_SEMANTICS( AATreeNode );

public:
  AATreeNode() : m_left( nullptr ), m_right( nullptr ), m_level( 1 ) { }
  ~AATreeNode() { }

  AATreeNode *get_left() const { return m_left; }
  void set_left( AATreeNode *left ) { m_left = left; }

  AATreeNode *get_right() const { return m_right; }
  void set_right( AATreeNode *right ) { m_right = right; }

  int get_level() const { return m_level; }
  void set_level( int level ) { m_level = level; }

  // Get effective level of left child.
  // This should only be called by AATreeImpl, where it's used
  // in the implementation of the remove() member function.
  int get_left_level() const { return m_left != nullptr ? m_left->get_level() : m_level; }

  // As above.
  int get_right_level() const { return m_right != nullptr ? m_right->get_level() : m_level; }

  // Get pointer to this node's left pointer.
  // Only AATreeImpl should  call this function.
  AATreeNode **get_ptr_to_left() { return &m_left; }

  // Get pointer to this node's right pointer.
  // Only AATreeImpl should  call this function.
  AATreeNode **get_ptr_to_right() { return &m_right; }

  bool is_leaf() const { return m_left == nullptr && m_right == nullptr; }
};

//! AA tree implementation.
//! Don't use this directly: instead, use AATree, parametized with
//! the actual tree node type.
class AATreeImpl {
public:
  //! Type of node comparison function: returns true IFF left node
  //! compares as less than right node
  typedef bool LessThanFn( const AATreeNode *left, const AATreeNode *right );

  //! Type of node contents copy function.
  //! This is used to handle the situation where a non-leaf node
  //! is deleted: we find an easy-to-remove node to be a "victim",
  //! copy the contents of the victim into the "deleted" node, and
  //! then remove the victim.
  typedef void NodeCopyFn( AATreeNode *from, AATreeNode *to );

  //! Node free function type
  typedef void FreeNodeFn( AATreeNode *node );

private:
  AATreeNode *m_root;
  LessThanFn *m_less_than_fn;
  NodeCopyFn *m_node_copy_fn;
  FreeNodeFn *m_free_node_fn;

  NO_VALUE_SEMANTICS( AATreeImpl );

  // Assume that the height of the tree will never be greater than this:
  // allows for using fixed-size arrays to keep track of nodes along
  // a path from root to leaf
  static constexpr const int MAX_HEIGHT = 32;

public:
  AATreeImpl( LessThanFn *less_than_fn, NodeCopyFn *node_copy_fn, FreeNodeFn *free_node_fn );
  ~AATreeImpl();

  bool insert( AATreeNode *node );
  AATreeNode *find( const AATreeNode &node ) const;
  bool contains( const AATreeNode &node ) const;
  bool remove( const AATreeNode &node );

private:
  static AATreeNode *skew( AATreeNode *t );
  static AATreeNode *split( AATreeNode *t );
  static void adjust_level( AATreeNode *t );
};

//! Balanced binary search tree class.
//! @tparam ActualNodeType the actual tree node type
template< typename ActualNodeType >
class AATree {
private:
  AATreeImpl m_impl;

  NO_VALUE_SEMANTICS( AATree );

public:
  //! Constructor.
  //! @param less_than_fn function to compare two tree nodes to determine
  //!                     whether the left node is less than the right node
  //! @param free_node_fn function to delete a tree node
  AATree( AATreeImpl::LessThanFn *less_than_fn, AATreeImpl *node_copy_fn, AATreeImpl::FreeNodeFn *free_node_fn )
    : m_impl( less_than_fn, node_copy_fn, free_node_fn )
  { }

  //! Destructor.
  ~AATree() { }

  //! Insert given node into the AATree.
  //! @param node the node to insert
  //! @return true if the node is inserted successfully, in which case
  //!         the AATree assumes ownership of it, or false if a node
  //!         comparing as equal already exists in the AATree,
  //!         in which case the node remains the caller's responsibility
  bool insert( ActualNodeType *node ) {
    return m_impl.insert( node );
  }

  //! Search for a node in the tree comparing as equal to the given one.
  //! @param node a node
  //! @return pointer to a tree nod equal to the given node,
  //!         or nullptr if the tree does not contain a node equal to
  //!         the given one
  ActualNodeType *find( const ActualNodeType &node ) const {
    return static_cast< ActualNodeType* >( m_impl.find( node ) );
  }

  //! Determine if the tree contains a node equal to the given one.
  //! @param node a node
  //! @return true if the tree contains a node equal to the given one,
  //!         false otherwise
  bool contains( const ActualNodeType &node ) const {
    return m_impl.contains( node );
  }

  //! Remove the node equal to the given one.
  //! If such a node is found, it is deleted using the free node function.
  //! @return true if a node was deleted, false if the tree did not
  //!         contain a node equal to the given one
  bool remove( const ActualNodeType &node ) {
    return m_impl.remove( node );
  }
};

} // end namespace dslib

#endif // DS_AATREE_H
