#include "Environment.hpp"

#include <stdexcept>
#include <utility>

Environment::Environment() {
  // The global scope is always present at the bottom of the stack.
  scopes.push_back({});
}

void Environment::pushScope() {
  scopes.push_back({});
}

void Environment::popScope() {

    if (scopes.size() <= 1) {
        throw std::runtime_error(
            "Cannot pop the global scope"
        );
    }

    scopes.pop_back();
}

void Environment::define(
    const std::string& name,
    const Value& value,
    VariableType type
) {
  scopes.back()[name] = Binding{value, type};
}

Environment::Binding* Environment::find(const std::string& name) {

  for (auto scope = scopes.rbegin(); scope != scopes.rend(); ++scope) {

    auto it = scope->find(name);

    if (it != scope->end()) {
      return &it->second;
    }
  }

  return nullptr;
}

const Environment::Binding* Environment::find(
    const std::string& name
) const {

  for (auto scope = scopes.rbegin(); scope != scopes.rend(); ++scope) {

    auto it = scope->find(name);

    if (it != scope->end()) {
      return &it->second;
    }
  }

  return nullptr;
}

Value Environment::get(const std::string& name) const {

  const Binding* binding = find(name);

  if (binding == nullptr) {
    throw std::runtime_error(
        "Undefined variable: " + name
    );
  }

  return binding->value;
}

void Environment::assign(const std::string& name, const Value& value) {

  Binding* existing = find(name);

  if (existing == nullptr) {
    throw std::runtime_error(
        "Undefined variable: " + name
    );
  }

  // The declared type does not change on reassignment; only the value does.
  existing->value = value;
}

VariableType Environment::declaredType(const std::string& name) const {

  const Binding* binding = find(name);

  if (binding == nullptr) {
    throw std::runtime_error(
        "Undefined variable: " + name
    );
  }

  return binding->type;
}