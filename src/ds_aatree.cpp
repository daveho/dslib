#include "ds_aatree.h"

namespace dslib {

AATreeImpl::AATreeImpl( LessThanFn *less_than_fn, CopyNodeFn *copy_node_fn, FreeNodeFn *free_node_fn )
  : m_root( nullptr )
  , m_less_than_fn( less_than_fn )
  , m_copy_node_fn( copy_node_fn )
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

bool AATreeImpl::remove( const AATreeNode &node ) {
  // Keep track of pointers that may need to be updated
  AATreeNode **path[ MAX_HEIGHT ];
  int path_len = 0;
  AATreeNode **link = &m_root;

  // Find a node equal to the given one
  while ( *link != nullptr ) {
    DS_ASSERT( path_len < MAX_HEIGHT );
    path[ path_len ] = link;
    ++path_len;

    if ( m_less_than_fn( *link, &node ) )
      link = (*link)->get_ptr_to_left(); // continue in left subtree
    else if ( !m_less_than_fn( &node, *link ) )
      break; // *link is pointing to a matching node
    else
      link = (*link)->get_ptr_to_right(); // continue in right subtree
  }

  if ( *link == nullptr )
    return false;  // the tree doesn't contain a matching node

  // Refer to the node *link points to as "t". There are three cases:
  // 1. If t points to a leaf, that node can be removed directly
  // 2. If t points to a node with a single child, that child
  //    becomes the root of the subtree that t was
  //    originally the root of, and t is removed
  // 3. Otherwise, leftmost node in t's right subtree is chosen as
  //    a "victim". The contents of the victim node are copied into
  //    t, and then the victim node is removed.
  AATreeNode *t = *link;
  if ( t->is_leaf() ) {
    // Case 1
    *link = nullptr;
    m_free_node_fn( t );
  } else if ( t->get_left() == nullptr ) {
    // Case 2 (left subtree is empty)
    *link = t->get_right();
    m_free_node_fn( t );
  } else if ( t->get_right() == nullptr ) {
    // Case 2 (right subtree is empty)
    *link = t->get_left();
    m_free_node_fn( t );
  } else {
    // Case 3

    DS_ASSERT( path_len < MAX_HEIGHT );
    link = t->get_ptr_to_right();
    path[ path_len ] = link;
    ++path_len;

    // Find the leftmost node in the subtree
    while ( (*link)->get_left() != nullptr ) {
      DS_ASSERT( path_len < MAX_HEIGHT );
      path[ path_len ] = link;
      ++path_len;
      link = (*link)->get_ptr_to_left();
    }

    DS_ASSERT( (*link)->get_left() == nullptr );

    AATreeNode *victim = *link;

    // Copy the contents of the victim to the deleted node
    m_copy_node_fn( victim, t );

    // The subtree rooted by the victim node is replaced by the
    // victim node's right subtree.
    *link = victim->get_right();
    m_free_node_fn( victim );
  }

  // Fix up all nodes
  while ( path_len > 0 ) {
    --path_len;
    link = path[ path_len ];
    adjust_level( *link );
    *link = skew( *link );
    *link = split( *link );
  }

  return true;
}

AATreeNode *AATreeImpl::skew( AATreeNode *t ) {
  if ( t == nullptr )
    return nullptr;

  AATreeNode *left = t->get_left();

  if ( left == nullptr )
    return t;

  if ( t->get_level() == left->get_level() ) {
    // t has a left child at the same level, so the left child  //
    // becomes the new root of this subtree, and t becomes its  //
    // right child.                                             //
    //                                                          //
    //            |             |                               //
    //            v             v                               //
    //   left <-- t            left -->  t                      //
    //  /   \      \   ==>    /         / \                     //
    // A     B      R        A         B   R                    //
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
    // There are two horizontal right links, so t's right node  //
    // needs to be pulled up.                                   //
    //                                                          //
    //      |                              |                    //
    //      v                              v                    //
    //      t -->  right --> x  ==>      right                  //
    //     /      /                     /     \                 //
    //    A      B                     t       x                //
    //                                / \                       //
    //                               A   B                      //
    t->set_right( right->get_left() );
    right->set_left( t );
    right->set_level( right->get_level() + 1 );
    return right;
  }

  return t;
}

void AATreeImpl::adjust_level( AATreeNode *t ) {
  if ( t == nullptr )
    return;

  // From Andersson's paper (p.3, "Deletion"):
  //   "If a pseudo-node is missing below p, i.e. if one of
  //   p's children is two levels below p, decrease the level of
  //   p by 1. If p's right child belonged to the same
  //   pseudo-node as p, we decrease the level of that node too."
  int t_level = t->get_level(),
     l_level = t->get_left_level(),
     r_level = t->get_right_level();

  bool has_right_child_at_same_level =
    t->get_right() != nullptr && r_level == t->get_level();

  if ( l_level < t_level - 1 || r_level < t_level - 1 ) {
    t->set_level( t_level - 1 );
    if ( has_right_child_at_same_level )
      t->get_right()->set_level( t_level - 1 );
  }
}

#ifdef DSLIB_CHECK_INTEGRITY
bool AATreeImpl::is_valid( AATreeNode *node, int expected_level ) {
  AATreeNode *left = node->get_left(), *right = node->get_right();

  // True leaf nodes must be at level 1
  if ( left == nullptr && right == nullptr )
    return node->get_level() == 1;

  // If there is a left child, it must be at the next lower level
  if ( left != nullptr )
    if ( !is_valid( left, expected_level - 1 ) )
      return false;

  if ( right == nullptr )
    return true; // no right subtree

  // Right child could be a level below the parent
  if ( right->get_level() == expected_level - 1 )
    return is_valid( right, expected_level - 1 );
  else {
    // Right node should be at same level as parent
    // (i.e., part of the same pseudo-node)
    if ( right->get_level() != expected_level )
      return false;

    // If the right child has a right child, it must be one level below
    if ( right->get_right() != nullptr )
      if ( right->get_right()->get_level() != expected_level - 1 )
        return false;

    return is_valid( right, expected_level );
  }
}
#endif

} // namespace dslib
