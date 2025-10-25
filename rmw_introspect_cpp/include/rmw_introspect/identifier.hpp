#ifndef RMW_INTROSPECT__IDENTIFIER_HPP_
#define RMW_INTROSPECT__IDENTIFIER_HPP_

// Identifier must be a single symbol that all compilation units reference
// Using extern ensures pointer equality works for identifier comparison
extern "C" const char *const rmw_introspect_cpp_identifier;

#endif // RMW_INTROSPECT__IDENTIFIER_HPP_
