#include "Parser.hpp"

#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens) {
}

Token Parser::current() const {
    if (position >= tokens.size()) {
        return {TokenType::EndOfFile, ""};
    }

    return tokens[position];
}

Token Parser::advance() {
    Token token = current();

    if (position < tokens.size()) {
        position++;
    }

    return token;
}

Program Parser::parse() {

  Program program;

  while (current().type != TokenType::EndOfFile) {
    program.statements.push_back(statement());
  }

  return program;
}

std::unique_ptr<Stmt> Parser::statement() {

    auto expr = expression();

    if (current().type != TokenType::Semicolon) {
        throw std::runtime_error(
            "Expected ';' after statement"
        );
    }

    advance();

    return std::make_unique<ExpressionStmt>(
        std::move(expr)
    );
}

std::unique_ptr<Expr> Parser::expression() {

    auto left = comparison();

    while (
        current().type == TokenType::EqualEqual ||
        current().type == TokenType::NotEqual
    ) {
        std::string op = advance().value;

        auto right = comparison();

        left = std::make_unique<BinaryExpr>(
            std::move(left),
            op,
            std::move(right)
        );
    }

    return left;
}

std::unique_ptr<Expr> Parser::comparison() {

    auto left = addition();

    while (
        current().type == TokenType::Less ||
        current().type == TokenType::LessEqual ||
        current().type == TokenType::Greater ||
        current().type == TokenType::GreaterEqual
    ) {
        std::string op = advance().value;

        auto right = addition();

        left = std::make_unique<BinaryExpr>(
            std::move(left),
            op,
            std::move(right)
        );
    }

    return left;
}

std::unique_ptr<Expr> Parser::addition() {

    auto left = term();

    while (
        current().type == TokenType::Plus ||
        current().type == TokenType::Minus
    ) {
        std::string op = advance().value;

        auto right = term();

        left = std::make_unique<BinaryExpr>(
            std::move(left),
            op,
            std::move(right)
        );
    }

    return left;
}

std::unique_ptr<Expr> Parser::term() {

    auto left = factor();

    while (
        current().type == TokenType::Star ||
        current().type == TokenType::Slash
    ) {
        std::string op = advance().value;

        auto right = factor();

        left = std::make_unique<BinaryExpr>(
            std::move(left),
            op,
            std::move(right)
        );
    }

    return left;
}

std::unique_ptr<Expr> Parser::factor() {
  return primary();
}

std::unique_ptr<Expr> Parser::number() {

    Token token = advance();

    return std::make_unique<NumberExpr>(
        std::stoi(token.value)
    );
}

std::unique_ptr<Expr> Parser::primary() {

    if (current().type == TokenType::Integer) {
        return number();
    }

    if (current().type == TokenType::Float) {
        return floatNumber();
    }

    if (current().type == TokenType::String) {
        return string();
    }

    if (
        current().type == TokenType::True ||
        current().type == TokenType::False
    ) {
        return boolean();
    }

    if (current().type == TokenType::Identifier) {

        std::string name = advance().value;

        if (current().type == TokenType::LeftParen) {

            advance();

            std::vector<std::unique_ptr<Expr>> arguments;

            if (current().type != TokenType::RightParen) {

                arguments.push_back(expression());

                while (current().type == TokenType::Comma) {
                    advance();
                    arguments.push_back(expression());
                }
            }

            if (current().type != TokenType::RightParen) {
                throw std::runtime_error(
                    "Expected ')'"
                );
            }

            advance();

            return std::make_unique<CallExpr>(
                std::move(name),
                std::move(arguments)
            );
        }

        throw std::runtime_error(
            "Expected '(' after function name"
        );
    }

    if (current().type == TokenType::LeftParen) {

        advance();

        auto expr = expression();

        if (current().type != TokenType::RightParen) {
            throw std::runtime_error(
                "Expected ')'"
            );
        }

        advance();

        return expr;
    }

    throw std::runtime_error(
        "Expected expression"
    );
}

std::unique_ptr<Expr> Parser::floatNumber() {

    Token token = advance();

    return std::make_unique<FloatExpr>(
        std::stod(token.value)
    );
}

std::unique_ptr<Expr> Parser::string() {

    Token token = advance();

    return std::make_unique<StringExpr>(
        token.value
    );
}

std::unique_ptr<Expr> Parser::boolean() {

    Token token = advance();

    return std::make_unique<BooleanExpr>(
        token.type == TokenType::True
    );
}
