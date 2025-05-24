# ww-dslib: the world's worst C++ data structure library

This project is intended to be a small collection of useful data
structures implemented in C++. It has a fairly specific set of
goals:

* suitable for embedded systems and OS kernels
* no recursion
* memory efficiency (all node-based collections are intrusive, so
  the node is the value)
* small code size (all data structure operations are separately
  compiled, not expanded from a template)
* reasonable ergnomics (templates provide a thin layer over
  the base data structure implementations to reduce, but not
  necessarily eliminate, the need for explicit typecasts in the
  application code)

## Status

This is totally incomplete and experimental.

A doubly-linked list implementation seems to work.

An AA-tree implementation has been begun but is not very far along.
