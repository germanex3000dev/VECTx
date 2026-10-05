#include "Interpreter.hpp"

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <iostream>
#include <utility>

namespace {

// Pushes a scope on construction and pops it on destruction. Used so that a
// runtime error thrown part way through a block cannot leave the environment
// with that block's scope still on the stack.
class ScopeGuard {
public:
    explicit ScopeGuard(Environment& environment)
        : environment(environment) {
        environment.pushScope();
    }

    ~ScopeGuard() {
        environment.popScope();
    }

    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

private:
    Environment& environment;
};

// Unwinds a function body back to the call that started it. A distinct
// exception type, so it cannot be confused with a runtime error, and so that a
// 'return' inside nested blocks still leaves the whole function.
struct ReturnSignal {
    Value value;
    bool hasValue;
};

// The zero of each type, used to give the declared return variable a starting
// value before the body runs.
Value defaultValue(VariableType type) {
    switch (type) {
        case VariableType::Integer: return Value(0);
        case VariableType::Float: return Value(0.0);
        case VariableType::Boolean: return Value(false);
        case VariableType::String: return Value(std::string());
        case VariableType::Any: return Value();
    }
    return Value();
}

const char* typeName(ValueType type) {
    switch (type) {
        case ValueType::Integer: return "int";
        case ValueType::Float: return "float";
        case ValueType::Boolean: return "bool";
        case ValueType::String: return "string";
        case ValueType::Null: return "null";
    }
    return "unknown";
}

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

// Reads one line and converts it to `type`, or throws.
Value readTypedLine(VariableType type) {

    std::string line;

    while (true) {

        if (!std::getline(std::cin, line)) {
            throw std::runtime_error(
                "conin() reached the end of input without a line"
            );
        }

        // An empty line is a real, empty string rather than a failure, so a
        // program can tell it apart from a missing line. Every other type
        // skips over blank input instead, since no value of those types can
        // be spelled with nothing at all.
        if (
            type == VariableType::String ||
            type == VariableType::Any
        ) {
            break;
        }

        if (line.find_first_not_of(" \t\r") != std::string::npos) {
            break;
        }
    }

    if (type == VariableType::String || type == VariableType::Any) {
        return Value(line);
    }

    if (type == VariableType::Boolean) {
        if (line == "true") return Value(true);
        if (line == "false") return Value(false);

        throw std::runtime_error(
            "conin(type: bool) expected 'true' or 'false', got \"" +
            line + "\""
        );
    }

    // Surrounding whitespace is trimmed, so " 7 " still reads as 7. The ends
    // are then checked by hand because std::stoll and std::stod happily read
    // a prefix and ignore the rest, so "12abc" would pass as 12.
    std::size_t first = line.find_first_not_of(" \t\r");

    if (first != std::string::npos) {
        line = line.substr(
            first, line.find_last_not_of(" \t\r") - first + 1
        );
    }

    std::size_t used = 0;

    try {
        if (type == VariableType::Integer) {

            long long value = std::stoll(line, &used);

            if (used != line.size()) {
                throw std::invalid_argument("trailing");
            }

            if (
                value < std::numeric_limits<int>::min() ||
                value > std::numeric_limits<int>::max()
            ) {
                throw std::out_of_range("out of int range");
            }

            return Value(static_cast<int>(value));
        }

        double value = std::stod(line, &used);

        if (used != line.size()) {
            throw std::invalid_argument("trailing");
        }

        return Value(value);
    } catch (const std::exception&) {
        throw std::runtime_error(
            std::string("conin(type: ") +
            (type == VariableType::Integer ? "int" : "float") +
            ") expected a number, got \"" + line + "\""
        );
    }
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

  // Registered first, so a function may call one declared later in the file.
  for (const auto& function : program.functions) {
    if (!functions.emplace(function->name, function.get()).second) {
      throw std::runtime_error(
          "Function already declared: " + function->name
      );
    }
  }

  for (const auto& stmt : program.statements) {
    execute(stmt.get());
  }
}

// Binds the arguments in a scope of their own, runs the body, and yields
// whatever 'return' produced. A call does not push a scope of its own beyond
// the body's block, which already has one.
Value Interpreter::callFunction(
    const FunctionStmt* function,
    const CallExpr* call
) {

    if (call->arguments.size() != function->parameters.size()) {
        throw std::runtime_error(
            function->name + "() expects " +
            std::to_string(function->parameters.size()) +
            " argument(s), got " +
            std::to_string(call->arguments.size())
        );
    }

    ScopeGuard scope(environment);

    for (std::size_t i = 0; i < function->parameters.size(); ++i) {

        const Parameter& parameter = function->parameters[i];

        Value argument = evaluate(call->arguments[i].get());

        if (!isOfType(argument, parameter.type)) {
            throw std::runtime_error(
                "Type mismatch in argument '" + parameter.name +
                "' of " + function->name
            );
        }

        environment.define(parameter.name, argument, parameter.type);
    }

    // The declared return variable starts at its type's zero, so a function
    // that returns early or never at all still yields something of the right
    // type.
    environment.define(
        function->returnName,
        defaultValue(function->returnType),
        function->returnType
    );

    bool returned = false;
    Value value;

    try {
        execute(function->body.get());
    } catch (const ReturnSignal& signal) {
        value = signal.value;
        returned = signal.hasValue;
    }

    // 'return;' and falling off the end both yield the return variable, so it
    // is read after the body rather than before the scope is popped.
    if (returned) {

        if (!isOfType(value, function->returnType)) {
            throw std::runtime_error(
                "Return type mismatch in " + function->name
            );
        }

        return value;
    }

    return environment.get(function->returnName);
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

    if (auto assignment = dynamic_cast<const AssignStmt*>(statement)) {

        executeAssignment(assignment);
        return;
    }

    if (auto block = dynamic_cast<const BlockStmt*>(statement)) {

        executeBlock(block);
        return;
    }

    if (auto conditional = dynamic_cast<const IfStmt*>(statement)) {

        executeIf(conditional);
        return;
    }

    if (auto returnStatement =
        dynamic_cast<const ReturnStmt*>(statement)) {

        executeReturn(returnStatement);
        return;
    }

    throw std::runtime_error(
        "Unknown statement type"
    );
}

void Interpreter::executeReturn(const ReturnStmt* statement) {

    Value value;

    // A bare 'return;' carries no value of its own, so hasValue is false and
    // the call falls back to the declared return variable.
    if (statement->value != nullptr) {
        value = evaluate(statement->value.get());
    }

    throw ReturnSignal{value, statement->value != nullptr};
}

void Interpreter::executeExpression(
    const ExpressionStmt* statement
) {
    evaluate(statement->expression.get());
}

void Interpreter::executeBlock(const BlockStmt* statement) {

    // The scope is released even if a statement throws.
    ScopeGuard scope(environment);

    for (const auto& stmt : statement->statements) {
        execute(stmt.get());
    }
}

void Interpreter::executeIf(const IfStmt* statement) {

    Value condition = evaluate(statement->condition.get());

    if (condition.type() != ValueType::Boolean) {
        throw std::runtime_error(
            std::string(
                "if condition must be a bool, got "
            ) + typeName(condition.type())
        );
    }

    if (condition.asBool()) {
        execute(statement->thenBranch.get());
        return;
    }

    if (statement->elseBranch != nullptr) {
        execute(statement->elseBranch.get());
    }
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

    environment.define(statement->name, value, statement->type);
}

void Interpreter::executeAssignment(const AssignStmt* statement) {

    // Look the variable up first, so assigning to a name that does not
    // exist reports that rather than evaluating the value for nothing.
    VariableType declared = environment.declaredType(statement->name);

    Value value = evaluate(statement->value.get());

    // A concretely typed variable keeps its type, so the new value has to
    // match. 'any' is dynamic and accepts a value of any type, which is how
    // an 'any' variable changes what it holds.
    if (
        declared != VariableType::Any &&
        !isOfType(value, declared)
    ) {
        throw std::runtime_error(
            "Type mismatch in assignment to '" +
            statement->name + "'"
        );
    }

    environment.assign(statement->name, value);
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
    return environment.get(expr->name);
}

Value Interpreter::evaluateCall(const CallExpr* expr) {

    auto function = functions.find(expr->name);

    if (function != functions.end()) {
        return callFunction(function->second, expr);
    }

    if (expr->name == "conin") {

        // conin(type: <type>) reads the line as that type. A type may not be
        // mixed with value arguments.
        if (
            expr->readType.has_value() &&
            !expr->arguments.empty()
        ) {
            throw std::runtime_error(
                "conin(type: ...) takes no arguments"
            );
        }

        if (expr->readType.has_value()) {
            return readTypedLine(*expr->readType);
        }

        if (!expr->arguments.empty()) {
            throw std::runtime_error(
                "conin() expects no arguments"
            );
        }

        return readTypedLine(VariableType::String);
    }

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

// && and || require bool operands, just like an if condition, and
// short-circuit: the right operand is only evaluated when the left one does
// not already decide the result.
Value Interpreter::evaluateLogical(const BinaryExpr* expr) {

    Value left = evaluate(expr->left.get());

    if (left.type() != ValueType::Boolean) {
        throw std::runtime_error(
            std::string(
                expr->op == BinaryOp::LogicalAnd
                    ? "'&&' requires a bool, got "
                    : "'||' requires a bool, got "
            ) + typeName(left.type())
        );
    }

    if (expr->op == BinaryOp::LogicalAnd && !left.asBool()) {
        return Value(false);
    }

    if (expr->op == BinaryOp::LogicalOr && left.asBool()) {
        return Value(true);
    }

    Value right = evaluate(expr->right.get());

    if (right.type() != ValueType::Boolean) {
        throw std::runtime_error(
            std::string(
                expr->op == BinaryOp::LogicalAnd
                    ? "'&&' requires a bool, got "
                    : "'||' requires a bool, got "
            ) + typeName(right.type())
        );
    }

    return Value(right.asBool());
}

Value Interpreter::evaluateBinary(const BinaryExpr* expr) {

    // The logical operators short-circuit, so they are handled before the
    // operands are evaluated: the right one may never be evaluated at all.
    if (
        expr->op == BinaryOp::LogicalAnd ||
        expr->op == BinaryOp::LogicalOr
    ) {
        return evaluateLogical(expr);
    }

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
