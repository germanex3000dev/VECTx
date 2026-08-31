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

    std::unique_ptr<Stmt> statement();

    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> addition();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();

    std::unique_ptr<Expr> number();
    std::unique_ptr<Expr> floatNumber();
    std::unique_ptr<Expr> string();
    std::unique_ptr<Expr> boolean();

    std::unique_ptr<Expr> primary();
};
