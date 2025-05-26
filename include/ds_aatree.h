#ifndef DS_AATREE_H
#define DS_AATREE_H

#include "ds_util.h"

namespace dslib {

//! Assume that the height of an AA-tree will never be greater than this:
//! allows for using fixed-size arrays to keep track of nodes along
//! a path from root to leaf.
const constexpr int AA_TREE_MAX_HEIGHT = 32;

class AATreeImpl;
class AATreeIterImpl;
class AATreePostfixIterImpl;
#ifdef DSLIB_CHECK_INTEGRITY
class TreePrintContext;
#endif

//! Intrusive AA tree node base class.
//! Your node type must derive from this class.
class AATreeNode {
private:
  AATreeNode *m_left, *m_right;
  int m_level;

  NO_VALUE_SEMANTICS( AATreeNode );

public:
  AATreeNode() : m_left( nullptr ), m_right( nullptr ), m_level( 1 ) { }
  ~AATreeNode() { }

  // Allow certain implementation classes direct access to
  // children pointers and level information
  friend class AATreeImpl;
  friend class AATreeIterImpl;
  friend class AATreePostfixIterImpl;
#ifdef DSLIB_CHECK_INTEGRITY
  friend class TreePrintContext;
#endif

private:
  AATreeNode *get_left() const { return m_left; }
  AATreeNode *get_right() const { return m_right; }
  int get_level() const { return m_level; }

  void set_left( AATreeNode *left ) { m_left = left; }
  void set_right( AATreeNode *right ) { m_right = right; }
  void set_level( int level ) { m_level = level; }
  AATreeNode **get_ptr_to_left() { return &m_left; }
  AATreeNode **get_ptr_to_right() { return &m_right; }
};

//! Fixed-size stack of pointers.
//! This is used to keep track of the path from the
//! root to a specific node in AATreeImpl::insert(),
//! AATreeImpl::remove(), and the two iterator implementations
//! (in-order and postorder.) You should not need to use this
//! directly.
class AATreePtrStackImpl {
private:
  void *m_stack[ AA_TREE_MAX_HEIGHT ];
  int m_num_items;

  // note that this class DOES have value semantics

public:
  AATreePtrStackImpl();
  ~AATreePtrStackImpl();

  bool is_empty() const;
  void push( void *p );
  void *top() const;
  void *pop();
};

//! Stack of pointers of specified pointer type.
//! This is used for the AA tree operations and iterator
//! implementations to keep track of the path from the root
//! to a specific node. Note that this class has
//! value semantics.
template< typename PtrType >
class AATreePtrStack {
private:
  AATreePtrStackImpl m_impl;

public:
  //! Constructor.
  AATreePtrStack() { }

  //! Destructor.
  ~AATreePtrStack() { }

  //! @return true if the stack is empty, false if not
  bool is_empty() const { return m_impl.is_empty(); }

  //! Push a pointer onto the stack.
  //! @param p the pointer to push on the stack
  void push( PtrType p ) { m_impl.push( static_cast< void* >( p ) ); }

  //! @return the top pointer on the stack, which must be non-empty
  PtrType top() const { return static_cast< PtrType >( m_impl.top() ); }

  //! Pop a pointer off the top of the stack, which must be non-empty.
  //! @return the pointer popped from the top of the stack
  PtrType pop() { return static_cast< PtrType >( m_impl.pop() ); }
};

// In-order iterator implementation.
// Don't use this directly: use AATreeIter instread,
// parametized with the actual node type.
class AATreeIterImpl {
private:
  AATreePtrStack< AATreeNode* > m_stack;
  const AATreeImpl *m_tree;
  
  // Note that this class DOES have value semantics

public:
  AATreeIterImpl();
  ~AATreeIterImpl();

  bool has_next() const;
  AATreeNode *next();

  friend class AATreeImpl;

private:
  void init( const AATreeImpl *tree );
};

//! Postfix iterator implementation.
//! Don't use this directly: use AATreePostfixIter instead,
//! parametized with the actual node type.
class AATreePostfixIterImpl {
private:
  AATreePtrStack< AATreeNode* > m_stack;
  const AATreeImpl *m_tree;

  // Note that this class DOES have value semantics

public:
  AATreePostfixIterImpl();
  ~AATreePostfixIterImpl();

  bool has_next() const;
  AATreeNode *next();

  friend class AATreeImpl;

private:
  void init( const AATreeImpl *tree );

  static AATreeNode *clean_ptr( AATreeNode *node );
  bool is_left_visited( AATreeNode *node );
  bool is_right_visited( AATreeNode *node );
  static AATreeNode *mark_left_visited( AATreeNode *node );
  static AATreeNode *mark_right_visited( AATreeNode *node );
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
  typedef void CopyNodeFn( AATreeNode *from, AATreeNode *to );

  //! Node free function type
  typedef void FreeNodeFn( AATreeNode *node );

private:
  AATreeNode *m_root;
  AATreeNode m_nil;
  LessThanFn *m_less_than_fn;
  CopyNodeFn *m_copy_node_fn;
  FreeNodeFn *m_free_node_fn;

  NO_VALUE_SEMANTICS( AATreeImpl );

public:
  AATreeImpl( LessThanFn *less_than_fn, CopyNodeFn *copy_node_fn, FreeNodeFn *free_node_fn );
  ~AATreeImpl();

  bool is_empty() const { return m_root == &m_nil; }

  bool insert( AATreeNode *node );
  AATreeNode *find( const AATreeNode &node ) const;
  bool contains( const AATreeNode &node ) const;
  bool remove( const AATreeNode &node );

  const AATreeNode *nil() const { return &m_nil; }

  AATreeIterImpl iterator() const;
  AATreePostfixIterImpl postfix_iterator() const;

#ifdef DSLIB_CHECK_INTEGRITY
  // Does AA-tree rooted at given node satisfy the AA-tree properties?
  bool is_valid( AATreeNode *node, int expected_level ) const;

  // Does the overall AA-tree satisfy the AA-tree properties?
  bool is_valid() const {
    if ( m_root == nullptr )
      return true;
    return is_valid( m_root, m_root->get_level() );
  }

  // Get pointer to root node
  AATreeNode *get_root() const { return m_root; }
#endif

private:
  AATreeNode *skew( AATreeNode *t );
  AATreeNode *split( AATreeNode *t );
  void adjust_level( AATreeNode *t );
};

//! In-order iterator over nodes in an AATree.
//! @tparam ActualNodeType the actual tree node type
template< typename ActualNodeType >
class AATreeIter {
private:
  AATreeIterImpl m_impl;

public:
  //! Constructor. This shouldn't be used directly:
  //! instead, call AATree::iterator().
  //! @param impl the underlying AATreeIterImpl positioned at the
  //!             first node
  AATreeIter( const AATreeIterImpl &impl )
    : m_impl( impl ) {

  }

  //! Destructor.
  ~AATreeIter() { }

  //! @return true if the iterator can return at least one more node,
  //!         false if there are no more nodes to return
  bool has_next() const {
    return m_impl.has_next();
  }

  //! Get the next node, and advance to the node that follows
  //! in order. Don't call this unless has_next() has returned true.
  //! @return the next node in the sequenbce
  ActualNodeType *next() {
    return static_cast< ActualNodeType* >( m_impl.next() );
  }
};

//! Postfix iterator over the nodes in an AATree.
//! @tparam ActualNodeType the actual tree node type
template< typename ActualNodeType >
class AATreePostfixIter {
private:
  AATreePostfixIterImpl m_impl;

public:
  //! Constructor. This shouldn't be used directly:
  //! instead, call AATree::postfix_iterator().
  //! @param impl the underlying AATreePostfixIterImpl positioned at the
  //!             first node in postfix order
  AATreePostfixIter( const AATreePostfixIterImpl &impl )
    : m_impl( impl ) {

  }

  //! Destructor.
  ~AATreePostfixIter() { }

  //! @return true if the iterator can return at least one more node,
  //!         false if there are no more nodes to return
  bool has_next() const {
    return m_impl.has_next();
  }

  //! Get the next node, and advance to the node that follows
  //! in postfix order. Don't call this unless has_next() has
  //! returned true.
  //! @return the next node in the sequenbce
  ActualNodeType *next() {
    return static_cast< ActualNodeType* >( m_impl.next() );
  }
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
  //! @param copy_node_fn function to copy the contents of a node to a different
  //!                     node (necessary when removing an interior node)
  //! @param free_node_fn function to delete a tree node
  AATree( AATreeImpl::LessThanFn *less_than_fn, AATreeImpl::CopyNodeFn *copy_node_fn, AATreeImpl::FreeNodeFn *free_node_fn )
    : m_impl( less_than_fn, copy_node_fn, free_node_fn )
  { }

  //! Destructor.
  ~AATree() { }

  //! Check whether the AATree is empty.
  //! @return true if the tree is empty, false if it has at least one node
  bool is_empty() const { return m_impl.is_empty(); }

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

  //! Get an iterator positioned at the first (i.e., overall least) node.
  //! @return an iterator positioned at the first (overall least) node
  AATreeIter< ActualNodeType > iterator() const {
    return AATreeIter< ActualNodeType >( m_impl.iterator() );
  }

  //! Get a postfix iterator positioned at the first node in postfix order.
  //! @return a postfix iterator positioned at the first node in postfix order
  AATreePostfixIter< ActualNodeType > postfix_iterator() const {
    return AATreePostfixIter< ActualNodeType >( m_impl.postfix_iterator() );
  }

#ifdef DSLIB_CHECK_INTEGRITY
  //! Check whether the tree satisfies the AST-tree properties
  //! @return true if the tree satisfies the AA-tree properties,
  //!         false if not
  bool is_valid() const {
    return m_impl.is_valid();
  }
#endif
};

} // end namespace dslib

#endif // DS_AATREE_H
