#pragma once

#include "AST.hpp"
#include "Environment.hpp"
#include "Value.hpp"

class Interpreter {
public:
    void execute(const Program& program);
    void execute(const Stmt* statement);

    Value evaluate(const Expr* expr);

private:
    Environment environment;

    void executeExpression(
        const ExpressionStmt* statement
    );

    void executeVariableDeclaration(
        const VariableDeclStmt* statement
    );

    Value evaluateNumber(const NumberExpr* expr);
    Value evaluateFloat(const FloatExpr* expr);
    Value evaluateString(const StringExpr* expr);
    Value evaluateBoolean(const BooleanExpr* expr);
    Value evaluateIsType(const IsTypeExpr* expr);

    Value evaluateVariable(const VariableExpr* expr);

    Value evaluateBinary(const BinaryExpr* expr);
    Value evaluateCall(const CallExpr* expr);
};
