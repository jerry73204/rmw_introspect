#ifndef RMW_INTROSPECT__VISIBILITY_CONTROL_H_
#define RMW_INTROSPECT__VISIBILITY_CONTROL_H_

#ifdef __cplusplus
extern "C"
{
#endif

// This logic was borrowed (then namespaced) from the examples on the gcc wiki:
//     https://gcc.gnu.org/wiki/Visibility

#if defined _WIN32 || defined __CYGWIN__
  #ifdef __GNUC__
    #define RMW_INTROSPECT_EXPORT __attribute__ ((dllexport))
    #define RMW_INTROSPECT_IMPORT __attribute__ ((dllimport))
  #else
    #define RMW_INTROSPECT_EXPORT __declspec(dllexport)
    #define RMW_INTROSPECT_IMPORT __declspec(dllimport)
  #endif
  #ifdef RMW_INTROSPECT_BUILDING_DLL
    #define RMW_INTROSPECT_PUBLIC RMW_INTROSPECT_EXPORT
  #else
    #define RMW_INTROSPECT_PUBLIC RMW_INTROSPECT_IMPORT
  #endif
  #define RMW_INTROSPECT_PUBLIC_TYPE RMW_INTROSPECT_PUBLIC
  #define RMW_INTROSPECT_LOCAL
#else
  #define RMW_INTROSPECT_EXPORT __attribute__ ((visibility("default")))
  #define RMW_INTROSPECT_IMPORT
  #if __GNUC__ >= 4
    #define RMW_INTROSPECT_PUBLIC __attribute__ ((visibility("default")))
    #define RMW_INTROSPECT_LOCAL  __attribute__ ((visibility("hidden")))
  #else
    #define RMW_INTROSPECT_PUBLIC
    #define RMW_INTROSPECT_LOCAL
  #endif
  #define RMW_INTROSPECT_PUBLIC_TYPE
#endif

#ifdef __cplusplus
}
#endif

#endif  // RMW_INTROSPECT__VISIBILITY_CONTROL_H_
