#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include "tctest.h"
#include "ds_aatree.h"

////////////////////////////////////////////////////////////////////////
// Integer tree node type for testing
////////////////////////////////////////////////////////////////////////

class IntAATreeNode : public AATreeNode< IntAATreeNode > {
private:
  int m_val;

  NO_VALUE_SEMANTICS( IntAATreeNode );

public:
  IntAATreeNode( int val = 0 ) : m_val( val ) { }
  ~IntAATreeNode() { }

  void set_val( int val ) { m_val = val; }
  int get_val() const { return m_val; }

  static void free_node_fn( IntAATreeNode *node );
  static bool less_than_fn( const IntAATreeNode *left, const IntAATreeNode *right );
};

void IntAATreeNode::free_node_fn( IntAATreeNode *node ) {
  delete node;
}

bool IntAATreeNode::less_than_fn( const IntAATreeNode *left, const IntAATreeNode *right ) {
  return left->m_val < right->m_val;  
}

////////////////////////////////////////////////////////////////////////
// Turn dslib assertions into tctest test failures
////////////////////////////////////////////////////////////////////////

void ds_assert_fail( const char *msg, const char *filename, int line ) {
  std::stringstream ss;
  ss << filename << ":" << line << ": " << msg;
  FAIL( ss.str().c_str() ); // uses siglongjmp to "throw" to tctest failure handling code
  for (;;);
}
////////////////////////////////////////////////////////////////////////
// Test fixture
////////////////////////////////////////////////////////////////////////

struct TestObjs {
  AATree< IntAATreeNode > itree;

  TestObjs()
    : itree( &IntAATreeNode::less_than_fn, &IntAATreeNode::free_node_fn )
  { }
};

////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////

// test fixture setup and cleanup
TestObjs *setup();
void cleanup( TestObjs *objs );
// test functions
void test_insert( TestObjs *objs );

////////////////////////////////////////////////////////////////////////
// Test program
////////////////////////////////////////////////////////////////////////

int main( int argc, char **argv ) {
  if ( argc > 1 )
    tctest_testname_to_execute = argv[1];

  TEST_INIT();

  TEST( test_insert );

  TEST_FINI();
}

////////////////////////////////////////////////////////////////////////
// Function implementations
////////////////////////////////////////////////////////////////////////

TestObjs *setup() {
  TestObjs *objs = new TestObjs;
  return objs;
}

void cleanup( TestObjs *objs ) {
  delete objs;
}

void test_insert( TestObjs *objs ) {
  auto &itree = objs->itree;

  const std::vector< int > VALS = { 16, 53, 3, 98, 79, 80, 17, 11, 42, 86 };
  for ( auto i = VALS.begin(); i != VALS.end(); ++i )
    itree.insert( new IntAATreeNode( *i ) );
  
  for ( int i = 0; i < 100; ++i ) {
    if ( std::find( VALS.begin(), VALS.end(), i ) != VALS.end() )
      ASSERT( itree.contains( i ) );
    else
      ASSERT( !itree.contains( i ) );
  }
}