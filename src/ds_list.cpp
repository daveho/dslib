// Copyright 2025, David H. Hovemeyer <david.hovemeyer@gmail.com>

// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// “Software”), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "ds_list.h"

namespace dslib {

ListImpl::ListImpl( FreeNodeFn *free_node_fn )
  : m_free_node_fn( free_node_fn ) {
  DS_ASSERT( m_head.get_prev() == nullptr );
  DS_ASSERT( m_tail.get_next() == nullptr );

  m_head.set_next( &m_tail );
  m_tail.set_prev( &m_head );
}

ListImpl::~ListImpl() {
  for ( auto p = get_first(); p != nullptr; ) {
    auto succ = next( p );
    m_free_node_fn( p );
    p = succ;
  }
}

bool ListImpl::is_empty() const {
  return m_head.get_next() == &m_tail;
}

ListNode *ListImpl::get_first() const {
  return is_empty() ? nullptr : m_head.get_next();
}

ListNode *ListImpl::get_last() const {
  return is_empty() ? nullptr : m_tail.get_prev();
}

void ListImpl::append( ListNode *node ) {
  DS_ASSERT( m_tail.get_prev() != nullptr );
  node->set_prev( m_tail.get_prev() );
  node->set_next( &m_tail );
  m_tail.get_prev()->set_next( node );
  m_tail.set_prev( node );
}

void ListImpl::prepend( ListNode *node ) {
  DS_ASSERT( m_head.get_next() != nullptr );
  node->set_prev( &m_head );
  node->set_next( m_head.get_next() );
  m_head.get_next()->set_prev( node );
  m_head.set_next( node );
}

void ListImpl::insert_before( ListNode *node_to_insert, ListNode *existing ) {
  node_to_insert->set_prev( existing->get_prev() );
  node_to_insert->set_next( existing );
  existing->get_prev()->set_next( node_to_insert );
  existing->set_prev( node_to_insert );
}

void ListImpl::insert_after( ListNode *node_to_insert, ListNode *existing ) {
  node_to_insert->set_prev( existing );
  node_to_insert->set_next( existing->get_next() );
  existing->get_next()->set_prev( node_to_insert );
  existing->set_next( node_to_insert );
}

void ListImpl::remove( ListNode *node_to_remove ) {
  auto pred = node_to_remove->get_prev(), succ = node_to_remove->get_next();
  pred->set_next( succ );
  succ->set_prev( pred );

  // For robustness, clear removed node's next and prev fields
  node_to_remove->set_prev( nullptr );
  node_to_remove->set_next( nullptr );
}

unsigned ListImpl::get_size() const {
  unsigned count = 0;
  for ( auto p = get_first(); p != nullptr; p = next( p ) )
    ++count;
  return count;
}

ListNode *ListImpl::next( ListNode *node ) const {
  auto succ = node->get_next();
  return ( succ == &m_tail ) ? nullptr : succ;
}

ListNode *ListImpl::prev( ListNode *node ) const {
  auto pred = node->get_prev();
  return ( pred == &m_head ) ? nullptr : pred;
}

} // end namespace dslib
