#ifndef DS_LIST_H
#define DS_LIST_H

#include "ds_util.h"

namespace dslib {

//! Intrusive list node base class.
class ListNode {
private:
  ListNode *m_prev, *m_next;

  NO_VALUE_SEMANTICS( ListNode );

public:
  //! Constructor.
  ListNode() : m_prev( nullptr ), m_next( nullptr ) { }

  //! Constructor from specified next and prev node pointers
  ListNode( ListNode *next, ListNode *prev )
    : m_prev( prev )
    , m_next( next ) { }
  ~ListNode() {}

  //! @return pointer to previous node (nullptr if there is no previous node)
  ListNode *get_prev() const { return m_prev; }

  //! Set the previous node pointer.
  //! @param prev previous node pointer to set
  void set_prev( ListNode *prev ) { m_prev = prev; }

  //! @return pointer to next node (nullptr if there is no next node)
  ListNode *get_next() const { return m_next; }
  
  //! Set the next node pointer.
  //! @param next next node pointer to set
  void set_next( ListNode *next ) { m_next = next; }
};

//! List header implementation class.
//! Don't use this directly: instead, use List, parametized with the
//! actual list node type, which should derive from ListNode.
class ListImpl {
public:
  //! Node free function type.
  typedef void FreeNodeFn( ListNode *node );

private:
  FreeNodeFn *m_free_node_fn;
  ListNode *m_head, *m_tail;

  NO_VALUE_SEMANTICS( ListImpl );

public:
  ListImpl( FreeNodeFn *free_node_fn );
  ~ListImpl();

  bool is_empty() const;
  ListNode *get_first() const;
  ListNode *get_last() const;
  void append( ListNode *node );
  void prepend( ListNode *node );
  void insert_before( ListNode *node_to_insert, ListNode *existing );
  void insert_after( ListNode *node_to_insert, ListNode *existing );
  void remove( ListNode *node_to_remove );
  unsigned get_size() const;

private:
  void add_initial_node( ListNode *node );
};

//! List class, storing a sequence of nodes.
//! @tparam ActualNodeType the list node type, which should derive
//!         from ListNode
template< typename ActualNodeType >
class List {
private:
  ListImpl m_impl;

  NO_VALUE_SEMANTICS( List );

public:
  List( ListImpl::FreeNodeFn *free_node_fn )
    : m_impl( free_node_fn ) { }
  ~List() { }

  //! Get the list node that follows the given one.
  //! @return the next list node, or nullptr if the given list
  //!         node is the tail of the list
  ActualNodeType *next( ActualNodeType *node ) const {
    return static_cast< ActualNodeType* >( node->get_next() );
  }

  //! Get the list node that follows the given one.
  //! @return the previous list node, or nullptr if the given list
  //!         node is the head of the list
  ActualNodeType *prev( ActualNodeType *node ) const {
    return static_cast< ActualNodeType *>( node->get_prev() );
  }

  //! @return true if list is empty, false if not
  bool is_empty() const { return m_impl.is_empty(); }

  //! @return pointer to the first node in the list
  ActualNodeType *get_first() const {
    return static_cast< ActualNodeType* >( m_impl.get_first() );
  }

  //! @return pointer to the last node in the list
  ActualNodeType *get_last() const {
    return static_cast< ActualNodeType* >( m_impl.get_last() );
  }

  //! Append given node to the list.
  //! The node becomes owned by the list (and will be
  //! freed by the list's destructor.)
  //! @param node the node to append
  void append( ActualNodeType *node ) {
    m_impl.append( node );
  }

  //! Prepend given node to the list.
  //! The node becomes owned by the list (and will be
  //! freed by the list's destructor.)
  //! @param node the node to append
  void prepend( ListNode *node ) {
    m_impl.prepend( node );
  }

  //! Insert a new node before an existing node.
  //! @param node_to_insert the node to insert
  //! @param existing an existing list node
  void insert_before( ListNode *node_to_insert, ListNode *existing ) {
    m_impl.insert_before( node_to_insert, existing );
  }

  //! Insert a new node after an existing node.
  //! @param node_to_insert the node to insert
  //! @param existing an existing list node
  void insert_after( ListNode *node_to_insert, ListNode *existing ) {
    m_impl.insert_after( node_to_insert, existing );
  }

  //! Remove a list node.
  //! The List gives up ownership of the removed node,
  //! so it's the caller's responsibility to free it.
  //! @param existing an existing list node
  void remove( ListNode *node_to_remove ) {
    m_impl.remove( node_to_remove );
  }

  //! @return the number of nodes in the list (note that this involves
  //           an O(N) traversal of the list nodes)
  unsigned get_size() const { return m_impl.get_size(); }
};

} // end namespace dslib

#endif // DS_LIST_H
