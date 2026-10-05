#pragma once

#include "AST.hpp"
#include "Environment.hpp"
#include "Value.hpp"

#include <string>
#include <unordered_map>

class Interpreter {
public:
    void execute(const Program& program);
    void execute(const Stmt* statement);

    Value evaluate(const Expr* expr);

void executeReturn(const ReturnStmt* statement);

    Value callFunction(
        const FunctionStmt* function,
        const CallExpr* call
    );

private:
    Environment environment;

    // Declared functions, by name. Populated before anything runs.
    std::unordered_map<std::string, const FunctionStmt*> functions;

    void executeExpression(
        const ExpressionStmt* statement
    );

    void executeVariableDeclaration(
        const VariableDeclStmt* statement
    );

    void executeAssignment(const AssignStmt* statement);

    void executeBlock(const BlockStmt* statement);

    void executeIf(const IfStmt* statement);

    Value evaluateNumber(const NumberExpr* expr);
    Value evaluateFloat(const FloatExpr* expr);
    Value evaluateString(const StringExpr* expr);
    Value evaluateBoolean(const BooleanExpr* expr);
    Value evaluateIsType(const IsTypeExpr* expr);

    Value evaluateVariable(const VariableExpr* expr);

    Value evaluateBinary(const BinaryExpr* expr);
    Value evaluateLogical(const BinaryExpr* expr);
    Value evaluateCall(const CallExpr* expr);
};
