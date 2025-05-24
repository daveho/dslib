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

  // Rebalance
  while ( path_len > 0 ) {
    --path_len;
    link = path[ path_len ];
    *link = skew( *link );
    *link = split( *link );
  }

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

AATreeNode *AATreeImpl::skew( AATreeNode *t ) {
  if ( t == nullptr )
    return nullptr;

  AATreeNode *left = t->get_left();

  if ( left == nullptr )
    return t;

  if ( t->get_level() == left->get_level() ) {
    //            |             |
    //            v             v
    //   left <-- t            left -->  t
    //  /   \      \   ==>    /         / \    :-)
    // A     B      R        A         B   R
    t->set_left( left->get_right() );
    left->set_right( t );
    return left;
  }

  return t;
}

AATreeNode *AATreeImpl::split( AATreeNode *t ) {
  if ( t == nullptr )
    return nullptr;

  AATreeNode *right = t->get_right();

  if ( right == nullptr )
    return t;

  AATreeNode *x =  right->get_right();

  if ( x == nullptr )
    return t;

  if ( t->get_level() == x->get_level() ) {
    // There are two horizontal right links, so t's right node
    // needs to be pulled up.
    //
    //      |                              |
    //      v                              v
    //      t -->  right --> x  ==>      right
    //     /      /                     /     \     :-)
    //    A      B                     t       x
    //                                / \           :-)
    //                               A   B
    t->set_right( right->get_left() );
    right->set_left( t );
    right->set_level( right->get_level() + 1 );
    return right;
  }

  return t;
}

} // namespace dslib
