#ifndef DS_LIST_H
#define DS_LIST_H

#include "ds_util.h"

//! Intrusive list node base class.
//! @tparam ActualNodeType the actual node type, which should
//!         be a subclass of ListNode
template< typename ActualNodeType >
class ListNode {
private:
  ActualNodeType *m_prev, *m_next;

  NO_VALUE_SEMANTICS( ListNode );

public:
  //! Constructor.
  ListNode() : m_prev( nullptr ), m_next( nullptr ) { }

  //! Constructor from specified next and prev node pointers
  ListNode( ActualNodeType *next, ActualNodeType *prev )
    : m_prev( prev )
    , m_next( next ) { }
  ~ListNode() {}

  //! @return pointer to previous node (nullptr if there is no previous node)
  ActualNodeType *get_prev() const { return m_prev; }

  //! Set the previous node pointer.
  //! @param prev previous node pointer to set
  void set_prev( ActualNodeType *prev ) { m_prev = prev; }

  //! @return pointer to next node (nullptr if there is no next node)
  ActualNodeType *get_next() const { return m_next; }
  
  //! Set the next node pointer.
  //! @param next next node pointer to set
  void set_next( ActualNodeType *next ) { m_next = next; }
};

//! List header class.
//! @tparam ActualNodeType the actual list node type, which should inherit
//!         from ListNode
template< typename ActualNodeType >
class List {
public:
  //! Node free function type.
  typedef void FreeNodeFn( ActualNodeType *node );

private:
  FreeNodeFn *m_free_node_fn;
  ActualNodeType *m_head, *m_tail;

  NO_VALUE_SEMANTICS( List );

public:
  //! Constructor.
  //! Initializes empty list.
  //! @param free_node_fn function called to free a node when the list is destroyed
  List( FreeNodeFn *free_node_fn )
    : m_free_node_fn( free_node_fn )
    , m_head( nullptr )
    , m_tail( nullptr ) { }

  //! Destructor.
  //! All of the nodes will be freed by the node free function.
  ~List() {
    for ( auto p = m_head; p != nullptr; ) {
      auto next = p->get_next();
      m_free_node_fn( p );
      p = next;
    }
  }

  //! @return true if list is empty, false if not
  bool is_empty() const {
    return m_head == nullptr;
  }

  //! @return pointer to the first node in the list
  ActualNodeType *get_first() const {
    DS_ASSERT( !is_empty() );
    return m_head;
  }

  //! @return pointer to the last node in the list
  ActualNodeType *get_last() const {
    DS_ASSERT( !is_empty() );
    return m_tail;
  }

  //! Append given node to the list.
  //! The node becomes owned by the list (and will be
  //! freed by the list's destructor.)
  //! @param node the node to append
  void append( ActualNodeType *node ) {
    if ( is_empty() )
      add_initial_node( node );
    else {
      DS_ASSERT( m_tail != nullptr );
      m_tail->set_next( node );
      node->set_prev( m_tail );
      node->set_next( nullptr );
      m_tail = node;
    }
  }

  //! Prepend given node to the list.
  //! The node becomes owned by the list (and will be
  //! freed by the list's destructor.)
  //! @param node the node to append
  void prepend( ActualNodeType *node ) {
    if ( is_empty() )
      add_initial_node( node );
    else {
      m_head->set_prev( node );
      node->set_next( m_head );
      node->set_prev( nullptr );
      m_head = node;
    }
  }

  //! Insert a new node before an existing node.
  //! @param node_to_insert the node to insert
  //! @param existing an existing list node
  void insert_before( ActualNodeType *node_to_insert, ActualNodeType *existing ) {
    if ( existing == m_head ) {
      DS_ASSERT( existing->get_prev() == nullptr );
      prepend( node_to_insert );
    } else {
      DS_ASSERT( existing->get_prev() != nullptr );
      ActualNodeType *prev = existing->get_prev();
      prev->set_next( node_to_insert );
      node_to_insert->set_prev( prev );
      node_to_insert->set_next( existing );
      existing->set_prev( node_to_insert );
    }
  }

  //! Insert a new node after an existing node.
  //! @param node_to_insert the node to insert
  //! @param existing an existing list node
  void insert_after( ActualNodeType *node_to_insert, ActualNodeType *existing ) {
    if ( existing == m_tail ) {
      DS_ASSERT( existing->get_next() == nullptr );
      append( node_to_insert );
    } else {
      DS_ASSERT( existing->get_next() != nullptr );
      ActualNodeType *next = existing->get_next();
      existing->set_next( node_to_insert );
      node_to_insert->set_prev( existing );
      node_to_insert->set_next( next );
      next->set_prev( node_to_insert );
    }
  }

  //! @return the number of nodes in the list (note that this involves
  //           an O(N) traversal of the list nodes)
  unsigned get_size() const {
    unsigned count = 0;
    for ( auto p = m_head; p != nullptr; p = p->get_next() )
      ++count;
    return count;
  }

private:
  void add_initial_node( ActualNodeType *node ) {
    DS_ASSERT( is_empty() );
    m_head = m_tail = node;
    node->set_next( nullptr );
    node->set_prev( nullptr );
  }
};

#endif // DS_LIST_H
