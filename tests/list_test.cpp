#include <iostream>
#include <string>
#include <sstream>
#include "tctest.h"
#include "ds_list.h"

////////////////////////////////////////////////////////////////////////
// Integer list for testing
////////////////////////////////////////////////////////////////////////

class IntListNode : public ListNode< IntListNode > {
private:
  int m_val;

  NO_VALUE_SEMANTICS( IntListNode );

public:
  IntListNode( int val ) : ListNode(), m_val( val ) { }

  int get_val() const { return m_val; }
  void set_val( int val ) { m_val = val; }

  static void free_int_list_node( IntListNode *node );
};

void IntListNode::free_int_list_node( IntListNode *node ) {
  delete node;
}

////////////////////////////////////////////////////////////////////////
// Turn dslib assertions into tctest test failures
////////////////////////////////////////////////////////////////////////

void dslib_assert_fail( const char *msg, const char *filename, int line ) {
  std::stringstream ss;
  ss << filename << ":" << line << ": " << msg;
  FAIL( ss.str().c_str() ); // uses siglongjmp to "throw" to tctest failure handling code
  for (;;);
}

////////////////////////////////////////////////////////////////////////
// Test fixture
////////////////////////////////////////////////////////////////////////

struct TestObjs {
  List< IntListNode > ilist;

  TestObjs() : ilist( &IntListNode::free_int_list_node ) {

  }
};

////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////

// test fixture setup and cleanup
TestObjs *setup();
void cleanup( TestObjs *objs );
// test functions
void test_empty_list( TestObjs *objs );
void test_append( TestObjs *objs );

////////////////////////////////////////////////////////////////////////
// Test program
////////////////////////////////////////////////////////////////////////

int main( int argc, char **argv ) {
  if ( argc > 1 )
    tctest_testname_to_execute = argv[1];

  TEST_INIT();

  TEST( test_empty_list );
  TEST( test_append );

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

void test_empty_list( TestObjs *objs ) {
  List< IntListNode > &ilist = objs->ilist;
  
  ASSERT( ilist.get_size() == 0 );
}

void test_append( TestObjs *objs ) {
  List< IntListNode > &ilist = objs->ilist;

  // Append nodes to list
  ilist.append( new IntListNode( 9 ) );
  ASSERT( ilist.get_size() == 1 );
  ilist.append( new IntListNode( 0 ) );
  ASSERT( ilist.get_size() == 2 );
  ilist.append( new IntListNode( 1 ) );
  ASSERT( ilist.get_size() == 3 );
  ilist.append( new IntListNode( 2 ) );
  ASSERT( ilist.get_size() == 4 );
  ilist.append( new IntListNode( 5 ) );
  ASSERT( ilist.get_size() == 5 );

  // verify contents
  auto p = ilist.get_first();
  ASSERT( p != nullptr );
  ASSERT( p->get_val() == 9 );
  p = p->get_next();
  ASSERT( p != nullptr );
  ASSERT( p->get_val() == 0 );
  p = p->get_next();
  ASSERT( p != nullptr );
  ASSERT( p->get_val() == 1 );
  p = p->get_next();
  ASSERT( p != nullptr );
  ASSERT( p->get_val() == 2 );
  p = p->get_next();
  ASSERT( p != nullptr );
  ASSERT( p->get_val() == 5 );
  p = p->get_next();

  // should be at end of list now
  ASSERT( p == nullptr );
}
