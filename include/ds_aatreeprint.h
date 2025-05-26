#ifndef DS_TREEPRINT_H
#define DS_TREEPRINT_H

#include <string>

namespace dslib {

#ifdef DSLIB_CHECK_INTEGRITY
class AATreeNode;

class AATreePrint {
public:
  AATreePrint();
  virtual ~AATreePrint();

  void print( AATreeNode *t ) const;

  virtual std::string node_contents_to_str( AATreeNode *t ) const = 0;
};
#endif // DSLIB_CHECK_INTEGRITY

} // end namespace dslib

#endif // DS_TREEPRINT_H
