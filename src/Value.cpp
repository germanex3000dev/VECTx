#include "Value.hpp"

#include <stdexcept>
#include <utility>

Value::Value()
  : valueType(ValueType::Null),
    data(std::monostate{}) {
}

Value::Value(int value)
  : valueType(ValueType::Integer),
    data(value) {
}

Value::Value(double value)
  : valueType(ValueType::Float),
    data(value) {
}

Value::Value(bool value)
  : valueType(ValueType::Boolean),
    data(value) {
}

Value::Value(std::string value)
  : valueType(ValueType::String),
    data(std::move(value)) {
}

ValueType Value::type() const {
  return valueType;
}

int Value::asInt() const {

  if (valueType != ValueType::Integer) {
    throw std::runtime_error(
        "Value is not an integer"
    );
  }

  return std::get<int>(data);
}

double Value::asFloat() const {

  if (valueType != ValueType::Float) {
    throw std::runtime_error(
        "Value is not a Float"
    );
  }

  return std::get<double>(data);
}

bool Value::asBool() const {

  if (valueType != ValueType::Boolean) {
    throw std::runtime_error(
        "Value is not a boolean"
    );
  }

  return std::get<bool>(data);
}

const std::string& Value::asString() const {

  if (valueType != ValueType::String) {
    throw std::runtime_error(
        "Value is not string"
    );
  }

  return std::get<std::string>(data);
}
