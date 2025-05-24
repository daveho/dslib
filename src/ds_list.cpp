#include "ds_list.h"

ListImpl::ListImpl( FreeNodeFn *free_node_fn )
  : m_free_node_fn( free_node_fn )
  , m_head( nullptr )
  , m_tail( nullptr ) { }

ListImpl::~ListImpl() {
  for ( auto p = m_head; p != nullptr; ) {
    auto next = p->get_next();
    m_free_node_fn( p );
    p = next;
  }
}

bool ListImpl::is_empty() const {
  return m_head == nullptr;
}

ListNode *ListImpl::get_first() const {
  DS_ASSERT( !is_empty() );
  return m_head;
}

ListNode *ListImpl::get_last() const {
  DS_ASSERT( !is_empty() );
  return m_tail;
}

void ListImpl::append( ListNode *node ) {
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

void ListImpl::prepend( ListNode *node ) {
  if ( is_empty() )
    add_initial_node( node );
  else {
    m_head->set_prev( node );
    node->set_next( m_head );
    node->set_prev( nullptr );
    m_head = node;
  }
}

void ListImpl::insert_before( ListNode *node_to_insert, ListNode *existing ) {
  if ( existing == m_head ) {
    DS_ASSERT( existing->get_prev() == nullptr );
    prepend( node_to_insert );
  } else {
    DS_ASSERT( existing->get_prev() != nullptr );
    ListNode *prev = existing->get_prev();
    prev->set_next( node_to_insert );
    node_to_insert->set_prev( prev );
    node_to_insert->set_next( existing );
    existing->set_prev( node_to_insert );
  }
}

void ListImpl::insert_after( ListNode *node_to_insert, ListNode *existing ) {
  if ( existing == m_tail ) {
    DS_ASSERT( existing->get_next() == nullptr );
    append( node_to_insert );
  } else {
    DS_ASSERT( existing->get_next() != nullptr );
    ListNode *next = existing->get_next();
    existing->set_next( node_to_insert );
    node_to_insert->set_prev( existing );
    node_to_insert->set_next( next );
    next->set_prev( node_to_insert );
  }
}

void ListImpl::remove( ListNode *node_to_remove ) {
  if ( node_to_remove == m_head ) {
    DS_ASSERT( node_to_remove->get_prev() == nullptr );
    m_head = m_head->get_next();
    if ( m_head == nullptr )
      m_tail = nullptr; // list becaome empty
    else
      m_head->set_prev( nullptr ); // new list head has no predecessor now
  } else if ( node_to_remove == m_tail ) {
    DS_ASSERT( node_to_remove->get_next() == nullptr );
    m_tail = m_tail->get_prev();
    if ( m_tail == nullptr )
      m_head = nullptr; // list became empty
    else
      m_tail->set_next( nullptr ); // new list tail has no successor now
  } else {
    // General case: node being removed has both a predecessor
    // and a successor
    ListNode *prev = node_to_remove->get_prev();
    ListNode *next = node_to_remove->get_next();

    // splice removed node out of the list
    prev->set_next( next );
    next->set_prev( prev );
  }

  // For robustness, clear removed node's next and prev fields
  node_to_remove->set_prev( nullptr );
  node_to_remove->set_next( nullptr );
}

unsigned ListImpl::get_size() const {
  unsigned count = 0;
  for ( auto p = m_head; p != nullptr; p = p->get_next() )
    ++count;
  return count;
}

void ListImpl::add_initial_node( ListNode *node ) {
  DS_ASSERT( is_empty() );
  m_head = m_tail = node;
  node->set_next( nullptr );
  node->set_prev( nullptr );
}
