#pragma once

#include "AST.hpp"
#include "Value.hpp"

#include <string>
#include <unordered_map>
#include <vector>

// Holds variables as a stack of scopes. The bottom scope is the global one;
// each block pushes a scope on entry and pops it on exit, so a variable
// declared inside a block disappears when the block ends.
//
// define() always writes to the innermost scope, creating the binding there.
// get() and assign() search outwards from the innermost scope, so a name
// declared in an enclosing scope stays reachable and writable from inside a
// block. That is what lets an inner scope update a variable it did not
// declare.
class Environment {
public:
  Environment();

  void pushScope();
  void popScope();

  void define(const std::string& name, const Value& value,
              VariableType type);
  Value get(const std::string& name) const;
  void assign(const std::string& name, const Value& value);

  // The type a variable was declared with, which is what reassignment is
  // checked against. Throws if the name is not defined.
  VariableType declaredType(const std::string& name) const;

private:
  // A binding remembers its declared type as well as its value, so that
  // assigning to it can be checked.
  struct Binding {
    Value value;
    VariableType type;
  };

  std::vector<std::unordered_map<std::string, Binding>> scopes;

  Binding* find(const std::string& name);
  const Binding* find(const std::string& name) const;
};
