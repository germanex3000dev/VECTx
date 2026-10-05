#pragma once

#include "AST.hpp"
#include "Token.hpp"

#include <memory>
#include <vector>

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    Program parse();

private:
    std::vector<Token> tokens;
    std::size_t position = 0;

    Token current() const;
    Token advance();

    VariableType typeName();

    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> variableDeclaration();
    std::unique_ptr<Stmt> assignment();
    std::unique_ptr<Stmt> ifStatement();
    std::unique_ptr<Stmt> block();
    std::unique_ptr<Stmt> body();

    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> logicalOr();
    std::unique_ptr<Expr> logicalAnd();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> addition();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();

    std::unique_ptr<Expr> isTypeCheck(std::unique_ptr<Expr> target);

    std::unique_ptr<Expr> number();
    std::unique_ptr<Expr> floatNumber();
    std::unique_ptr<Expr> string();
    std::unique_ptr<Expr> boolean();

    std::unique_ptr<Expr> primary();
};
