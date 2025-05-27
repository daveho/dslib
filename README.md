# ww-dslib: the world's worst C++ data structure library

This project is intended to be a small collection of useful data
structures implemented in C++. It has a fairly specific set of
goals:

* suitable for embedded systems and OS kernels
* no recursion
* no exceptions
* no RTTI
* memory efficiency (all node-based collections are intrusive, so
  the node is the value)
* minimal stack use (operations will, in general, not require more
  than a few dozen pointers worth of stack context, so they should
  be ok to call from threads with small runtime stacks)
* small code size (all data structure operations are separately
  compiled, not expanded from a template)
* reasonable ergnomics (templates provide a thin layer over
  the base data structure implementations to reduce, but not
  necessarily eliminate, the need for explicit typecasts in the
  application code)

## Status

This is somewhat incomplete and experimental.

A doubly-linked list implementation seems to work.

An [AA-tree](https://user.it.uu.se/~arneande/ps/simp.pdf) implementation
seems to be functional, but I can't guarantee that there are no bugs.

## How do I use it?

There's no real documentation yet. The best examples of using the
library are probably the unit tests:

* [list\_test.cpp](tests/list_test.cpp)
* [aatree\_test.cpp](tests/aatree_test.cpp)

## License

The library is availble under the terms of the MIT license:
see [LICENSE.txt](LICENSE.txt).

## WTF?

Email questions, complaints, etc. to <mailto:daveho@cs.jhu.edu>
