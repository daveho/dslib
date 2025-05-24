#include "ds_aatree.h"

namespace dslib {

AATreeImpl::AATreeImpl( LessThanFn *less_than_fn, FreeNodeFn *free_node_fn )
  : m_root( nullptr )
  , m_less_than_fn( less_than_fn )
  , m_free_node_fn( free_node_fn ) {
}

AATreeImpl::~AATreeImpl() {
  // TODO: free nodes
}

bool AATreeImpl::insert( AATreeNode *node ) {
  // The node should be in its initial state
  DS_ASSERT( node->get_left() == nullptr );
  DS_ASSERT( node->get_right() == nullptr );
  DS_ASSERT( node->get_level() == 1 );

  // Keep track of pointers that may need to be updated
  AATreeNode **path[ MAX_HEIGHT ];
  int path_len = 0;
  AATreeNode **link = &m_root;

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

AATreeNode *AATreeImpl::find( const AATreeNode &node ) const {
  AATreeNode *p = m_root;
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

bool AATreeImpl::contains( const AATreeNode &node ) const {
  return find( node ) != nullptr;
}

} // namespace dslib
