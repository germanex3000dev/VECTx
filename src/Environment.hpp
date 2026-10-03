#pragma once

#include "Value.hpp"

#include <string>
#include <unordered_map>

class Environment {
public:
  void define(const std::string& name, const Value& value);
  Value get(const std::string& name) const;
  void assign(const std::string& name, const Value& value);

private:
  std::unordered_map<std::string, Value> variables;
};
