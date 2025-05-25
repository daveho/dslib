#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <random>
#include "tctest.h"
#include "ds_aatree.h"

////////////////////////////////////////////////////////////////////////
// Integer tree node type for testing
////////////////////////////////////////////////////////////////////////

class IntAATreeNode : public dslib::AATreeNode {
private:
  int m_val;

  NO_VALUE_SEMANTICS( IntAATreeNode );

public:
  IntAATreeNode( int val = 0 ) : m_val( val ) { }
  ~IntAATreeNode() { }

  void set_val( int val ) { m_val = val; }
  int get_val() const { return m_val; }

  static void free_node_fn( dslib::AATreeNode *node );
  static void copy_node_fn( dslib::AATreeNode *from, dslib::AATreeNode *to );
  static bool less_than_fn( const dslib::AATreeNode *left, const dslib::AATreeNode *right );
};

void IntAATreeNode::free_node_fn( dslib::AATreeNode *node ) {
  delete static_cast< IntAATreeNode* >( node );
}

void IntAATreeNode::copy_node_fn( dslib::AATreeNode *from_, dslib::AATreeNode *to_ ) {
  IntAATreeNode *from = static_cast< IntAATreeNode* >( from_ );
  IntAATreeNode *to = static_cast< IntAATreeNode* >( to_ );

  to->set_val( from->get_val() );
}

bool IntAATreeNode::less_than_fn( const dslib::AATreeNode *left_, const dslib::AATreeNode *right_ ) {
  //return left->m_val < right->m_val;  
  const IntAATreeNode *left = static_cast< const IntAATreeNode* >( left_ );
  const IntAATreeNode *right = static_cast< const IntAATreeNode* >( right_ );
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
  dslib::AATree< IntAATreeNode > itree;

  TestObjs()
    : itree( &IntAATreeNode::less_than_fn, &IntAATreeNode::copy_node_fn, &IntAATreeNode::free_node_fn )
  { }
};

// some arbitrary test data
const std::vector< int > TEST_VALS = { 16, 53, 3, 98, 79, 80, 17, 11, 42, 86 };

////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////

// test fixture setup and cleanup
TestObjs *setup();
void cleanup( TestObjs *objs );
// test functions
void test_insert( TestObjs *objs );
void test_insert_many( TestObjs *objs );
void test_remove_one( TestObjs *objs );
void test_remove( TestObjs *objs );

////////////////////////////////////////////////////////////////////////
// Test program
////////////////////////////////////////////////////////////////////////

int main( int argc, char **argv ) {
  if ( argc > 1 )
    tctest_testname_to_execute = argv[1];

  TEST_INIT();

  TEST( test_insert );
  TEST( test_insert_many );
  TEST( test_remove_one );
  TEST( test_remove );

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

  for ( auto i = TEST_VALS.begin(); i != TEST_VALS.end(); ++i ) {
    itree.insert( new IntAATreeNode( *i ) );
    ASSERT( itree.is_valid() );
  }
  
  for ( int i = 0; i < 100; ++i ) {
    if ( std::find( TEST_VALS.begin(), TEST_VALS.end(), i ) != TEST_VALS.end() )
      ASSERT( itree.contains( i ) );
    else
      ASSERT( !itree.contains( i ) );
  }
}

void test_insert_many( TestObjs *objs ) {
  auto rng = std::default_random_engine();
  std::vector<int> vals;
  for ( int i = 0; i < 100000; ++i )
    vals.push_back( i );
  std::shuffle( vals.begin(), vals.end(), rng );

  auto &itree = objs->itree;

  for ( auto i = vals.begin(); i != vals.end(); ++i )
    itree.insert( new IntAATreeNode( *i ) );
  ASSERT( itree.is_valid() );  

  for ( auto i = vals.begin(); i != vals.end(); ++i )
    ASSERT( itree.contains( IntAATreeNode( *i ) ) );
}

void test_remove_one( TestObjs *objs ) {
  auto &itree = objs->itree;
  
  itree.insert( new IntAATreeNode( 42 ) );
  ASSERT( itree.contains( IntAATreeNode( 42 ) ) );
  bool removed = itree.remove( IntAATreeNode( 42 ) );
  ASSERT( removed );
}

void test_remove( TestObjs *objs ) {
  auto &itree = objs->itree;
  
  for ( auto i = TEST_VALS.begin(); i != TEST_VALS.end(); ++i )
    itree.insert( new IntAATreeNode( *i ) );

  for ( auto i = TEST_VALS.begin(); i != TEST_VALS.end(); ++i ) {
    bool removed = itree.remove( IntAATreeNode( *i ) );
    ASSERT( removed );
    ASSERT( !itree.contains( IntAATreeNode( *i ) ) );
    ASSERT( itree.is_valid() );
  }
}
