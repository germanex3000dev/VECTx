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

Value Interpreter::evaluateBinary(const BinaryExpr* expr) {

    Value left = evaluate(expr->left.get());
    Value right = evaluate(expr->right.get());

    if (
        left.type() != ValueType::Integer ||
        right.type() != ValueType::Integer
    ) {
        throw std::runtime_error(
            "Binary arithmetic currently requires integers"
        );
    }

    int lhs = left.asInt();
    int rhs = right.asInt();

    switch (expr->op) {

        case '+':
            return Value(lhs + rhs);

        case '-':
            return Value(lhs - rhs);

        case '*':
            return Value(lhs * rhs);

        case '/':
            if (rhs == 0) {
                throw std::runtime_error(
                    "Division by zero"
                );
            }

            return Value(lhs / rhs);

        default:
            throw std::runtime_error(
                "Unknown binary operator"
            );
    }
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
