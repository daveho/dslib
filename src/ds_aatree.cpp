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

#include <cstdint>
#include "ds_aatree.h"

namespace dslib {

////////////////////////////////////////////////////////////////////////
// AATreeImpl implementation
////////////////////////////////////////////////////////////////////////

AATreeImpl::AATreeImpl( LessThanFn *less_than_fn, CopyNodeFn *copy_node_fn, FreeNodeFn *free_node_fn )
  : m_root( nullptr )
  , m_less_than_fn( less_than_fn )
  , m_copy_node_fn( copy_node_fn )
  , m_free_node_fn( free_node_fn ) {
  // The special level-0 "nil" node is pointed to by all
  // "missing" level-1 links.
  m_nil.set_level( 0 );
  m_root = &m_nil;
}

AATreeImpl::~AATreeImpl() {
  // It should be completely safe to delete the nodes in
  // postfix order (this should eliminate any possibility
  // of using a node after it has been deleted)
  AATreePostfixIterImpl it = postfix_iterator();
  while ( it.has_next() ) {
    AATreeNode *node = it.next();
    m_free_node_fn( node );
  }
}

bool AATreeImpl::insert( AATreeNode *node ) {
  // The node should be in its initial state
  DS_ASSERT( node->get_left() == nullptr );
  DS_ASSERT( node->get_right() == nullptr );
  DS_ASSERT( node->get_level() == 1 );

  // Keep track of pointers that may need to be updated
  AATreePtrStack< AATreeNode** > path;
  AATreeNode **link = &m_root;

  while ( *link != &m_nil ) {
    path.push( link );

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

  // Make the nil node the left and right child of the
  // inserted node
  node->set_left( &m_nil );
  node->set_right( &m_nil );

  // Rebalance
  while ( !path.is_empty() ) {
    link = path.pop();
    *link = skew( *link );
    *link = split( *link );
  }

  return true;
}

AATreeNode *AATreeImpl::find( const AATreeNode &node ) const {
  AATreeNode *p = m_root;
  while ( p != &m_nil ) {
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
  AATreePtrStack< AATreeNode** > path;
  AATreeNode **link = &m_root;

  // Find a node equal to the given one
  while ( *link != &m_nil ) {
    path.push( link );

    if ( m_less_than_fn( &node, *link ) )
      // Node we're searching for is less than *link,
      // so continue in the left subtree
      link = (*link)->get_ptr_to_left();
    else if ( !m_less_than_fn( *link, &node ) )
       // *link is pointing to a matching node
      break;
    else
      // Node we're searching for is greater than the
      // current node, so continue in right subtree 
      link = (*link)->get_ptr_to_right();
  }

  if ( *link == &m_nil )
    return false;  // the tree doesn't contain a matching node

  // Refer to the node *link points to as "t". There are three cases:
  //
  // 1. If t points to a true leaf, that node can be removed directly
  // 2. If t points to a node with a single child, that child
  //    becomes the root of the subtree that t was
  //    originally the root of, and t is removed
  // 3. Otherwise, leftmost node in t's right subtree is chosen as
  //    a "victim". The contents of the victim node are copied into
  //    t, and then the victim node is removed.
  AATreeNode *t = *link;
  if ( t->get_left() == &m_nil && t->get_right() == &m_nil ) {
    // Case 1
    *link = &m_nil;
    m_free_node_fn( t );
  } else if ( t->get_left() == &m_nil ) {
    // Case 2 (left subtree is empty)
    *link = t->get_right();
    m_free_node_fn( t );
  } else if ( t->get_right() == &m_nil ) {
    // Case 2 (right subtree is empty)
    *link = t->get_left();
    m_free_node_fn( t );
  } else {
    // Case 3
    path.push( link );

    // Go to right subtree
    link = (*link)->get_ptr_to_right();

    // Find the leftmost node in the subtree
    while ( (*link)->get_left() != &m_nil ) {
      path.push( link );
      link = (*link)->get_ptr_to_left();
    }

    // Leftmost node in t's right subtree is the "victim"
    AATreeNode *victim = *link;

    // Copy the contents of the victim to the deleted node
    m_copy_node_fn( victim, t );

    // The subtree rooted by the victim node is replaced by the
    // victim node's right subtree.
    DS_ASSERT( victim != &m_nil );
    DS_ASSERT( victim->get_left() != nullptr );
    DS_ASSERT( victim->get_right() != nullptr );
    *link = victim->get_right();

    // Now we can delete the victim node
    m_free_node_fn( victim );
  }

  // Fix up all nodes
  while ( !path.is_empty() ) {
    link = path.pop();
    adjust_level( *link );
    *link = skew( *link );
    *link = split( *link );
  }

  return true;
}

AATreeIterImpl AATreeImpl::iterator() const {
  AATreeIterImpl it;
  it.init( this );
  return it;
}

AATreePostfixIterImpl AATreeImpl::postfix_iterator() const {
  AATreePostfixIterImpl it;
  it.init( this );
  return it;
}

AATreeNode *AATreeImpl::skew( AATreeNode *t ) {
  if ( t == &m_nil )
    return &m_nil;

  AATreeNode *left = t->get_left();

  if ( left == &m_nil )
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
  if ( t == &m_nil )
    return &m_nil;

  AATreeNode *right = t->get_right();

  if ( right == &m_nil )
    return t;

  AATreeNode *x = right->get_right();

  if ( x == &m_nil )
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
  if ( t == &m_nil )
    return;

  // From Andersson's paper (p.3, "Deletion"):
  //   "If a pseudo-node is missing below p, i.e. if one of
  //   p's children is two levels below p, decrease the level of
  //   p by 1. If p's right child belonged to the same
  //   pseudo-node as p, we decrease the level of that node too."

  DS_ASSERT( t->get_left() != nullptr );
  DS_ASSERT( t->get_right() != nullptr );

  AATreeNode *left = t->get_left(), *right = t->get_right();

  int t_level = t->get_level(),
      l_level = left->get_level(),
      r_level = right->get_level();
  
  bool r_at_same_level = ( t_level == r_level );

  if ( l_level == t_level-2 || r_level == t_level-2 ) {
    t->set_level( t_level - 1 );
    if ( r_at_same_level )
      right->set_level( t_level - 1 );
  }
}

#ifdef DSLIB_CHECK_INTEGRITY
bool AATreeImpl::is_valid( AATreeNode *node, int expected_level ) const {
  AATreeNode *left = node->get_left(), *right = node->get_right();

  // True leaf nodes must be at level 1
  if ( left == &m_nil && right == &m_nil )
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

////////////////////////////////////////////////////////////////////////
// AATreePtrStackImpl implementation
////////////////////////////////////////////////////////////////////////

AATreePtrStackImpl::AATreePtrStackImpl()
  : m_num_items( 0 ) {

}

AATreePtrStackImpl::~AATreePtrStackImpl() {

}

bool AATreePtrStackImpl::is_empty() const {
  return m_num_items <= 0;
}

void AATreePtrStackImpl::push( void *p ) {
  DS_ASSERT( m_num_items < AA_TREE_MAX_HEIGHT );
  m_stack[ m_num_items ] = p;
  ++m_num_items;
}

void *AATreePtrStackImpl::top() const {
  DS_ASSERT( !is_empty() );
  return m_stack[ m_num_items - 1 ];
}

void *AATreePtrStackImpl::pop() {
  DS_ASSERT( !is_empty() );
  --m_num_items;
  return m_stack[ m_num_items ];
}

////////////////////////////////////////////////////////////////////////
// AATreeIterImpl implementation
////////////////////////////////////////////////////////////////////////

AATreeIterImpl::AATreeIterImpl()
  : m_tree( nullptr ) {
  // Note that AATreeImpl is a friend class, and has
  // responsibility for initializing the stack and the
  // m_tree pointer
}

AATreeIterImpl::~AATreeIterImpl() {

}

bool AATreeIterImpl::has_next() const {
  DS_ASSERT( m_tree != nullptr );
  return !m_stack.is_empty();
}

AATreeNode *AATreeIterImpl::next() {
  DS_ASSERT( has_next() );

  // Get the current node (the one to return)
  AATreeNode *node = m_stack.top();

  // Advance to the next node (if there is one.)
  // Cases:
  //
  // 1. If there is a right child, the leftmost child in the right
  //    subtree is next
  // 2. If the current node is a left child of its parent, the
  //    parent is next
  // 3. Otherwise, go up, traversing all right child links.
  //    The first node reachable via a left child link is next.

  if ( node->get_right() != m_tree->nil() ) {
    // Case 1
    AATreeNode *next = node->get_right();
    m_stack.push( next );
    while ( next->get_left() != m_tree->nil() ) {
      next = next->get_left();
      m_stack.push( next );
    }
    return node;
  }

  // We're done with the subtree rooted at the
  // current node, so go up to parent
  m_stack.pop();

  if ( m_stack.is_empty() )
    // Done with the entire tree, next call to has_next() will return false
    return node;

  AATreeNode *parent = m_stack.top();
  if ( node == parent->get_left() )
    return node; // Case 2, immediate parent is the next node to visit
  
  // Case 3: traverse all upwards right links
  DS_ASSERT( node == parent->get_right() );
  DS_ASSERT( !m_stack.is_empty() );
  DS_ASSERT( m_stack.top() == parent );

  // In the following loop, "parent" is the node most recently popped
  // off the stack
  m_stack.pop();
  while ( !m_stack.is_empty() ) {
    AATreeNode *pp = m_stack.top();
    if ( pp->get_left() == parent )
      // pp was reached via a left link, so it is the next node,
      // and it's on the top of the stack, so we're done
      break;
    
    DS_ASSERT( pp->get_right() == parent );
    // pp was reached via a right link, so now it's "parent",
    // and we continue up
    parent = pp;
    m_stack.pop();
  }

  return node;
}

void AATreeIterImpl::init( const AATreeImpl *tree ) {
  m_tree = tree;

  // Start with the left-most node in the tree
  AATreeNode *n = m_tree->get_root();
  while ( n != m_tree->nil() ) {
    m_stack.push( n );
    n = n->get_left();
  }
}

////////////////////////////////////////////////////////////////////////
// AATreePostfixIterImpl implementation
////////////////////////////////////////////////////////////////////////

// We use the two least-significant bits of node pointers to keep
// track of whether the left subtree has been visited yet.

constexpr const uintptr_t LEFT_VISITED = 0x1;
constexpr const uintptr_t RIGHT_VISITED = 0x2;

// Mask to get the "clean" pointer
constexpr const uintptr_t CLEAN_PTR_MASK = ~( LEFT_VISITED|RIGHT_VISITED );

AATreePostfixIterImpl::AATreePostfixIterImpl()
  : m_tree( nullptr ) {
  // As with AATreeIterImpl, the AATreeImpl object will set up the
  // initial stack
}

AATreePostfixIterImpl::~AATreePostfixIterImpl() {

}

bool AATreePostfixIterImpl::has_next() const {
  DS_ASSERT( m_tree != nullptr );
  return !m_stack.is_empty();
}

AATreeNode *AATreePostfixIterImpl::next() {
  DS_ASSERT( has_next() );
  AATreeNode *cur = m_stack.pop();

  // The current node should not have any unvisited descendants
  DS_ASSERT( is_left_visited( cur ) );
  DS_ASSERT( is_right_visited( cur ) );

  // It's now safe to "clean" the pointer to the current node
  cur = clean_ptr( cur );

  // If the stack is not empty, then there are more nodes to visit
  if ( !m_stack.is_empty() ) {
    // Find the next node to visit

    // Go up to the parent, marking the completion of the visitation
    // of its left or right subtree, as appropriate
    AATreeNode *parent = m_stack.pop();
    if ( cur == clean_ptr( parent )->get_left() )
      m_stack.push( mark_left_visited( parent ) );
    else {
      DS_ASSERT( cur == clean_ptr( parent )->get_right() );
      m_stack.push( mark_right_visited( parent ) );
    }

    parent = m_stack.top();

    // It MUST be the case that the left subtree has been completely
    // visited, otherwise we would not have returned to the parent yet,
    // since the entire left subtree is always visited before the entire
    // right subtree.
    DS_ASSERT( is_left_visited( parent ) );

    // Check whether the parent's right subtree has been visited yet.
    // If so, great, the parent is the next node to be visited in the
    // postfix traversal. If the right subtree has NOT been visited
    // yet, find the next leaf to visit in the right subtree.

    if ( !is_right_visited( parent ) ) {
      AATreeNode *n = clean_ptr( parent )->get_right();
      DS_ASSERT( n != m_tree->nil() );
      while ( n != m_tree->nil() ) {
        m_stack.push( n );
        AATreeNode *left = n->get_left();
        n = ( left != m_tree->nil() ) ? left : n->get_right();
      }
    }
  }

  return cur;
}

void AATreePostfixIterImpl::init( const AATreeImpl *tree ) {
  m_tree = tree;

  // Start with the leftmost leaf, meaning that we traverse
  // to a leaf from the root PREFERRING left links, but taking
  // right links if they are the only option
  AATreeNode* n = m_tree->get_root();
  while ( n != m_tree->nil() ) {
    m_stack.push( n );
    AATreeNode *left = n->get_left();
    n = ( left != m_tree->nil() ) ? left : n->get_right();
  }

  // Either the tree is empty, or the first node visited should
  // be a true leaf
  DS_ASSERT( m_stack.is_empty() ||
             ( m_stack.top()->get_left() == m_tree->nil() &&
               m_stack.top()->get_right() == m_tree->nil() ) );
}

AATreeNode *AATreePostfixIterImpl::clean_ptr( AATreeNode *node ) {
  uintptr_t ptr_val = reinterpret_cast< uintptr_t >( node );
  return reinterpret_cast< AATreeNode* >( ptr_val & CLEAN_PTR_MASK );
}

bool AATreePostfixIterImpl::is_left_visited( AATreeNode *node ) {
  // If there is no left child, then trivially it has already
  // been visited
  if ( clean_ptr( node )->get_left() == m_tree->nil() )
    return true;

  // Check whether the LEFT_VISITED bit is set
  uintptr_t ptr_val = reinterpret_cast< uintptr_t >( node );
  return ( ptr_val & LEFT_VISITED ) != 0;
}

bool AATreePostfixIterImpl::is_right_visited( AATreeNode *node ) {
  // If there is no right child, then trivially it has already
  // been visitde
  if ( clean_ptr( node )->get_right() == m_tree->nil() )
    return true;

  // Check whether the RIGHT_VISITED bit is set
  uintptr_t ptr_val = reinterpret_cast< uintptr_t >( node );
  return ( ptr_val & RIGHT_VISITED ) != 0;
}

AATreeNode *AATreePostfixIterImpl::mark_left_visited( AATreeNode *node ) {
  uintptr_t ptr_val = reinterpret_cast< uintptr_t >( node );
  return reinterpret_cast< AATreeNode* >( ptr_val | LEFT_VISITED );
}

AATreeNode *AATreePostfixIterImpl::mark_right_visited( AATreeNode *node ) {
  uintptr_t ptr_val = reinterpret_cast< uintptr_t >( node );
  return reinterpret_cast< AATreeNode* >( ptr_val | RIGHT_VISITED );
}

} // namespace dslib
