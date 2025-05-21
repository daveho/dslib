#ifndef DS_UTIL_H
#define DS_UTIL_H

// Convenience for declaring copy ctor and assignment
// operator as nonexistent
#define NO_VALUE_SEMANTICS(Type) \
  Type( const Type & ) = delete; \
  Type &operator=( const Type & ) = delete

#ifndef NDEBUG
// The project using dslib should define this function
void ds_assert_fail( const char *msg, const char *filename, int line ) __attribute__ ((noreturn));

#define DS_ASSERT( expr ) \
if ( !(expr) ) \
  ds_assert_fail( "Assertion failed: " #expr, __FILE__, __LINE__ )
#else
#define DS_ASSERT( expr )
#endif

#endif // DS_UTIL_H
