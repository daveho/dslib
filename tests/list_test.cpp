#include <iostream>
#include <string>
#include <sstream>
#include "tctest.h"
#include "list.h"

class IntListNode;

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

void dslib_assert_fail( const char *msg, const char *filename, int line ) {
  std::stringstream ss;
  ss << filename << ":" << line << ": " << msg;
  FAIL( ss.str().c_str() );
  for (;;);
}

struct TestObjs {
  List< IntListNode > ilist;

  TestObjs() : ilist( &IntListNode::free_int_list_node ) {

  }
};

TestObjs *setup();
void cleanup( TestObjs *objs );
void test_empty_list( TestObjs *objs );

int main( int argc, char **argv ) {
  if ( argc > 1 )
    tctest_testname_to_execute = argv[1];

  TEST_INIT();

  TEST( test_empty_list );

  TEST_FINI();
}

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
