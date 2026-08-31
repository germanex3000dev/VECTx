#include "Interpreter.hpp"

#include <stdexcept>
#include <iostream>

void Interpreter::execute(const Program& program) {

  for (const auto& stmt : program.statements) {
    execute(stmt.get());
  }
}

void Interpreter::execute(const Stmt* statement) {

    if (auto expression =
        dynamic_cast<const ExpressionStmt*>(statement)) {

        executeExpression(expression);
        return;
    }

    throw std::runtime_error(
        "Unknown statement type"
    );
}

void Interpreter::executeExpression(
    const ExpressionStmt* statement
) {
    evaluate(statement->expression.get());
}

Value Interpreter::evaluate(const Expr* expr) {

    if (auto number = dynamic_cast<const NumberExpr*>(expr)) {
        return evaluateNumber(number);
    }

    if (auto binary = dynamic_cast<const BinaryExpr*>(expr)) {
        return evaluateBinary(binary);
    }

    if (auto call = dynamic_cast<const CallExpr*>(expr)) {
      return evaluateCall(call);
    }

    if (auto number = dynamic_cast<const NumberExpr*>(expr)) {
      return evaluateNumber(number);
    }

    if (auto floating = dynamic_cast<const FloatExpr*>(expr)) {
      return evaluateFloat(floating);
    }

    if (auto string = dynamic_cast<const StringExpr*>(expr)) {
      return evaluateString(string);
    }

    if (auto boolean = dynamic_cast<const BooleanExpr*>(expr)) {
    return evaluateBoolean(boolean);
    }

    if (auto binary = dynamic_cast<const BinaryExpr*>(expr)) {
      return evaluateBinary(binary);
    }

    if (auto call = dynamic_cast<const CallExpr*>(expr)) {
      return evaluateCall(call);
    }

    throw std::runtime_error(
        "Unknown expression type"
    );
}

Value Interpreter::evaluateNumber(const NumberExpr* expr) {
    return Value(expr->value);
}

Value Interpreter::evaluateCall(const CallExpr* expr) {

    if (expr->name == "conout") {

        if (expr->arguments.size() != 1) {
            throw std::runtime_error(
                "conout() expects exactly one argument"
            );
        }

        Value value = evaluate(
            expr->arguments[0].get()
        );

        switch (value.type()) {

            case ValueType::Integer:
                std::cout << value.asInt();
                break;

            case ValueType::Float:
                std::cout << value.asFloat();
                break;

            case ValueType::Boolean:
                std::cout << (
                    value.asBool() ? "true" : "false"
                );
                break;

            case ValueType::String:
                std::cout << value.asString();
                break;

            case ValueType::Null:
                std::cout << "null";
                break;
        }

        std::cout << '\n';

        return Value();
    }

    throw std::runtime_error(
        "Unknown function: " + expr->name
    );
}

namespace {

bool isNumber(const Value& v) {
    return v.type() == ValueType::Integer ||
           v.type() == ValueType::Float;
}

double asNumber(const Value& v) {
    if (v.type() == ValueType::Integer) {
        return static_cast<double>(v.asInt());
    }
    if (v.type() == ValueType::Float) {
        return v.asFloat();
    }
    throw std::runtime_error("Value is not numeric");
}

bool evalEquality(const std::string& op, const Value& left, const Value& right) {
    if (left.type() != right.type()) {
        return op == "!=";
    }

    switch (left.type()) {
        case ValueType::Integer:
            return op == "=="
                ? left.asInt() == right.asInt()
                : left.asInt() != right.asInt();

        case ValueType::Float:
            return op == "=="
                ? left.asFloat() == right.asFloat()
                : left.asFloat() != right.asFloat();

        case ValueType::Boolean:
            return op == "=="
                ? left.asBool() == right.asBool()
                : left.asBool() != right.asBool();

        case ValueType::String:
            return op == "=="
                ? left.asString() == right.asString()
                : left.asString() != right.asString();

        default:
            return op == "==";
    }
}

bool evalComparison(const std::string& op, double lhs, double rhs) {
    if (op == "<") return lhs < rhs;
    if (op == "<=") return lhs <= rhs;
    if (op == ">") return lhs > rhs;
    if (op == ">=") return lhs >= rhs;
    throw std::runtime_error("Unknown comparison operator");
}

} // namespace

Value Interpreter::evaluateBinary(const BinaryExpr* expr) {

    Value left = evaluate(expr->left.get());
    Value right = evaluate(expr->right.get());

    const std::string& op = expr->op;

    if (op == "==" || op == "!=") {
        return Value(evalEquality(op, left, right));
    }

    if (op == "<" || op == "<=" || op == ">" || op == ">=") {
        if (!isNumber(left) || !isNumber(right)) {
            throw std::runtime_error(
                "Comparison operators require numeric operands"
            );
        }
        return Value(
            evalComparison(op, asNumber(left), asNumber(right))
        );
    }

    if (!isNumber(left) || !isNumber(right)) {
        throw std::runtime_error(
            "Arithmetic operators require numeric operands"
        );
    }

    double lhs = asNumber(left);
    double rhs = asNumber(right);

    if (op == "+") return Value(lhs + rhs);
    if (op == "-") return Value(lhs - rhs);
    if (op == "*") return Value(lhs * rhs);

    if (op == "/") {
        if (rhs == 0) {
            throw std::runtime_error(
                "Division by zero"
            );
        }
        return Value(lhs / rhs);
    }

    throw std::runtime_error(
        "Unknown binary operator: " + op
    );
}

Value Interpreter::evaluateFloat(const FloatExpr* expr) {
    return Value(expr->value);
}

Value Interpreter::evaluateString(const StringExpr* expr) {
    return Value(expr->value);
}

Value Interpreter::evaluateBoolean(const BooleanExpr* expr) {
    return Value(expr->value);
}
