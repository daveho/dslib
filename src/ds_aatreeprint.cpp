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

#include <iostream>
#include <vector>
#include <utility>
#include <cassert>
#include "ds_aatree.h"
#include "ds_aatreeprint.h"

namespace dslib {

// Tree printing is only supported if DSLIB_CHECK_INTEGRITY
// is enabled
#ifdef DSLIB_CHECK_INTEGRITY

// pair of (index, num siblings):
// index is which sibling we're currently printing
typedef std::pair< int, int > StackItem;

struct TreePrintContext {
  std::vector< StackItem > stack;
  const AATreePrint *tp_obj;

  TreePrintContext( const AATreePrint *tp_obj_ )
    : tp_obj( tp_obj_ ) { }

  void pushctx( int nsibs );
  void popctx();
  void print_node( AATreeNode *n, const std::string &child = "" );
};

void TreePrintContext::pushctx( int nsibs_ ) {
  stack.push_back( { 0, nsibs_ } );
}

void TreePrintContext::popctx() {
  stack.pop_back();
}

void TreePrintContext::print_node( AATreeNode *n, const std::string &child ) {
  int depth = int( stack.size() );
  DS_ASSERT( depth > 0 );
  for ( int i = 1; i < depth; i++ ) {
    if ( i == depth-1 ) {
      std::cout << "+--";
    } else {
      int level_index = stack[i].first;
      int level_nsibs = stack[i].second;
      if ( level_index < level_nsibs ) {
        std::cout << "|  ";
      } else {
        std::cout << "   ";
      }
    }
  }

  // Print info about this node
  std::cout << child;
  std::cout << n->get_level() << "," << tp_obj->node_contents_to_str( n );
  std::cout << "\n";
  stack[depth-1].first++;

  int nkids = 0;
  if ( n->get_left() != nullptr )
    ++nkids;
  if ( n->get_right() != nullptr )
    ++nkids;
  pushctx(nkids);
  if ( n->get_left() != nullptr )
    print_node( n->get_left(), "L:" );
  if ( n->get_right() != nullptr )
    print_node( n->get_right(), "R:" );
  popctx();
}

AATreePrint::AATreePrint() {
}

AATreePrint::~AATreePrint() {
}

void AATreePrint::print( AATreeNode *t ) const {
  TreePrintContext ctx( this );
  ctx.pushctx( 1 );
  ctx.print_node( t );
}

#endif // DSLIB_CHECK_INTEGRITY

} // end namespace dslib
