#include "Interpreter.hpp"

#include <limits>
#include <stdexcept>
#include <iostream>

namespace {

// Applies an operator that preserves the integer type. Computed in 64 bits and
// range-checked, because signed overflow is undefined behaviour and silently
// wrapping an int is a worse answer than refusing to compute it.
int integerArithmetic(BinaryOp op, int lhs, int rhs) {

    long long result;

    switch (op) {
        case BinaryOp::Add:
            result = static_cast<long long>(lhs) + rhs;
            break;

        case BinaryOp::Subtract:
            result = static_cast<long long>(lhs) - rhs;
            break;

        case BinaryOp::Multiply:
            result = static_cast<long long>(lhs) * rhs;
            break;

        default:
            throw std::runtime_error(
                "Operator does not preserve the integer type"
            );
    }

    if (
        result < std::numeric_limits<int>::min() ||
        result > std::numeric_limits<int>::max()
    ) {
        throw std::runtime_error(
            "Integer overflow in arithmetic"
        );
    }

    return static_cast<int>(result);
}

// Maps a runtime value onto a declared variable type. Every concrete type
// requires an exact match; 'any' is the dynamic type, so it accepts any
// value that actually carries data.
bool isOfType(const Value& value, VariableType type) {
    switch (type) {
        case VariableType::Integer:
            return value.type() == ValueType::Integer;
        case VariableType::Float:
            return value.type() == ValueType::Float;
        case VariableType::Boolean:
            return value.type() == ValueType::Boolean;
        case VariableType::String:
            return value.type() == ValueType::String;
        case VariableType::Any:
            return value.type() != ValueType::Null;
    }
    return false;
}

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

bool evalEquality(BinaryOp op, const Value& left, const Value& right) {
    if (left.type() != right.type()) {
        return op == BinaryOp::NotEqual;
    }

    switch (left.type()) {
        case ValueType::Integer:
            return op == BinaryOp::Equal
                ? left.asInt() == right.asInt()
                : left.asInt() != right.asInt();

        case ValueType::Float:
            return op == BinaryOp::Equal
                ? left.asFloat() == right.asFloat()
                : left.asFloat() != right.asFloat();

        case ValueType::Boolean:
            return op == BinaryOp::Equal
                ? left.asBool() == right.asBool()
                : left.asBool() != right.asBool();

        case ValueType::String:
            return op == BinaryOp::Equal
                ? left.asString() == right.asString()
                : left.asString() != right.asString();

        default:
            return op == BinaryOp::Equal;
    }
}

bool evalComparison(BinaryOp op, double lhs, double rhs) {
    switch (op) {
        case BinaryOp::Less: return lhs < rhs;
        case BinaryOp::LessEqual: return lhs <= rhs;
        case BinaryOp::Greater: return lhs > rhs;
        case BinaryOp::GreaterEqual: return lhs >= rhs;
        default: throw std::runtime_error(
            "Unknown comparison operator"
        );
    }
}

} // namespace

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

    if (auto declaration =
        dynamic_cast<const VariableDeclStmt*>(statement)) {

        executeVariableDeclaration(declaration);
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

void Interpreter::executeVariableDeclaration(
    const VariableDeclStmt* statement
) {
    Value value = evaluate(statement->initializer.get());

    // 'any' is dynamic: it imposes no static constraint, so whatever the
    // initializer produced is stored as-is and keeps its runtime type.
    if (
        statement->type != VariableType::Any &&
        !isOfType(value, statement->type)
    ) {
        throw std::runtime_error(
            "Type mismatch in declaration of '" +
            statement->name + "'"
        );
    }

    enviroment.define(statement->name, value);
}

Value Interpreter::evaluate(const Expr* expr) {

    if (auto variable = dynamic_cast<const VariableExpr*>(expr)) {
        return evaluateVariable(variable);
    }

    if (auto number = dynamic_cast<const NumberExpr*>(expr)) {
        return evaluateNumber(number);
    }

    if (auto binary = dynamic_cast<const BinaryExpr*>(expr)) {
        return evaluateBinary(binary);
    }

    if (auto call = dynamic_cast<const CallExpr*>(expr)) {
      return evaluateCall(call);
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

    if (auto isType = dynamic_cast<const IsTypeExpr*>(expr)) {
      return evaluateIsType(isType);
    }

    throw std::runtime_error(
        "Unknown expression type"
    );
}

Value Interpreter::evaluateNumber(const NumberExpr* expr) {
    return Value(expr->value);
}

Value Interpreter::evaluateVariable(const VariableExpr* expr) {
    return enviroment.get(expr->name);
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

    switch (expr->op) {

        case BinaryOp::Equal:
        case BinaryOp::NotEqual:
            return Value(
                evalEquality(expr->op, left, right)
            );

        case BinaryOp::Less:
        case BinaryOp::LessEqual:
        case BinaryOp::Greater:
        case BinaryOp::GreaterEqual:
            if (!isNumber(left) || !isNumber(right)) {
                throw std::runtime_error(
                    "Comparison operators require numeric operands"
                );
            }
            return Value(
                evalComparison(
                    expr->op, asNumber(left), asNumber(right)
                )
            );

        case BinaryOp::Add:
        case BinaryOp::Subtract:
        case BinaryOp::Multiply:
        case BinaryOp::Divide: {

            if (!isNumber(left) || !isNumber(right)) {
                throw std::runtime_error(
                    "Arithmetic operators require numeric operands"
                );
            }

            // int op int stays an int. Division is the exception and always
            // widens, because 5 / 2 has no integer answer.
            if (
                expr->op != BinaryOp::Divide &&
                left.type() == ValueType::Integer &&
                right.type() == ValueType::Integer
            ) {
                return Value(
                    integerArithmetic(
                        expr->op,
                        left.asInt(),
                        right.asInt()
                    )
                );
            }

            double lhs = asNumber(left);
            double rhs = asNumber(right);

            switch (expr->op) {

                case BinaryOp::Add:
                    return Value(lhs + rhs);

                case BinaryOp::Subtract:
                    return Value(lhs - rhs);

                case BinaryOp::Multiply:
                    return Value(lhs * rhs);

                case BinaryOp::Divide:
                    if (rhs == 0) {
                        throw std::runtime_error(
                            "Division by zero"
                        );
                    }
                    return Value(lhs / rhs);

                default:
                    throw std::runtime_error(
                        "Unknown arithmetic operator"
                    );
            }
        }

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

Value Interpreter::evaluateIsType(const IsTypeExpr* expr) {

    Value value = evaluate(expr->target.get());

    return Value(isOfType(value, expr->expectedType));
}
