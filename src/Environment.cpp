#include "Environment.hpp"

#include <stdexcept>

void Environment::define(const std::string& name, const Value& value) {
  variables[name] = value;
}

Value Environment::get(const std::string& name) const {
  auto it = variables.find(name);

  if (it == variables.end()) {
    throw std::runtime_error("Undefined variable: " + name);
  }

  return it->second;
}

void Environment::assign(const std::string& name, const Value& value) {
  auto it = variables.find(name);

  if (it == variables.end()) {
    throw std::runtime_error("Undefined variable: " + name);
  }

  it->second = value;
} 
