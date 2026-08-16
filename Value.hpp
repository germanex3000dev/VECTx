#pragma once

#include <string>
#include <variant>

enum class ValueType {
  Integer,
  Float,
  Boolean,
  String, 
  Null
};

class Value {
public:
  Value();
  explicit Value(int value);
  explicit Value(double value);
  explicit Value(bool value);
  explicit Value(std::string value);

  ValueType type() const;

  int asInt() const;
  double asFloat() const;
  bool asBool() const;
  const std::string& asString() const;

private:
  ValueType valueType;

  std::variant<
    std::monostate,
    int,
    double,
    bool,
    std::string
  > data;
};
