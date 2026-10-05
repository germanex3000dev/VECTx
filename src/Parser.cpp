#include "Parser.hpp"

#include <stdexcept>

namespace {

BinaryOp toBinaryOp(TokenType type) {
    switch (type) {
        case TokenType::Plus:
            return BinaryOp::Add;
        case TokenType::Minus:
            return BinaryOp::Subtract;
        case TokenType::Star:
            return BinaryOp::Multiply;
        case TokenType::Slash:
            return BinaryOp::Divide;
        case TokenType::EqualEqual:
            return BinaryOp::Equal;
        case TokenType::NotEqual:
            return BinaryOp::NotEqual;
        case TokenType::Less:
            return BinaryOp::Less;
        case TokenType::LessEqual:
            return BinaryOp::LessEqual;
        case TokenType::Greater:
            return BinaryOp::Greater;
        case TokenType::GreaterEqual:
            return BinaryOp::GreaterEqual;
        case TokenType::AndAnd:
            return BinaryOp::LogicalAnd;
        case TokenType::OrOr:
            return BinaryOp::LogicalOr;
        default:
            throw std::runtime_error(
                "Not a binary operator"
            );
    }
}

} // namespace

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

VariableType Parser::typeName() {

    switch (current().type) {
        case TokenType::IntType:
            return VariableType::Integer;
        case TokenType::FloatType:
            return VariableType::Float;
        case TokenType::StringType:
            return VariableType::String;
        case TokenType::BoolType:
            return VariableType::Boolean;
        case TokenType::AnyType:
            return VariableType::Any;
        default:
            throw std::runtime_error(
                "Expected a type name"
            );
    }
}

Program Parser::parse() {

  Program program;

  while (current().type != TokenType::EndOfFile) {
    program.statements.push_back(statement());
  }

  return program;
}

std::unique_ptr<Stmt> Parser::statement() {

    if (current().type == TokenType::Var) {
        return variableDeclaration();
    }

    if (current().type == TokenType::If) {
        return ifStatement();
    }

    // A bare identifier followed by '=' is a reassignment. '==' is a
    // comparison, so it is not mistaken for one.
    if (current().type == TokenType::Identifier &&
        position + 1 < tokens.size() &&
        tokens[position + 1].type == TokenType::Equal) {
        return assignment();
    }

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

std::unique_ptr<Stmt> Parser::ifStatement() {

    advance(); // Consume 'if'

    if (current().type != TokenType::LeftParen) {
        throw std::runtime_error(
            "Expected '(' after 'if'"
        );
    }

    advance();

    auto condition = expression();

    if (current().type != TokenType::RightParen) {
        throw std::runtime_error(
            "Expected ')' after an if condition"
        );
    }

    advance();

    auto thenBranch = body();

    std::unique_ptr<Stmt> elseBranch;

    if (current().type == TokenType::Else) {

        advance(); // Consume 'else'

        // 'else if' is parsed as a nested if, which is also what makes
        // chains of else if work.
        if (current().type == TokenType::If) {
            elseBranch = ifStatement();
        } else {
            elseBranch = body();
        }
    }

    return std::make_unique<IfStmt>(
        std::move(condition),
        std::move(thenBranch),
        std::move(elseBranch)
    );
}

// The body of an if or else clause: a braced block, or a single statement.
std::unique_ptr<Stmt> Parser::body() {

    if (current().type == TokenType::LeftBrace) {
        return block();
    }

    return statement();
}

std::unique_ptr<Stmt> Parser::block() {

    advance(); // Consume '{'

    std::vector<std::unique_ptr<Stmt>> statements;

    while (current().type != TokenType::RightBrace) {

        if (current().type == TokenType::EndOfFile) {
            throw std::runtime_error(
                "Expected '}' to close a block"
            );
        }

        statements.push_back(statement());
    }

    advance(); // Consume '}'

    return std::make_unique<BlockStmt>(
        std::move(statements)
    );
}

std::unique_ptr<Stmt> Parser::assignment() {

    std::string name = advance().value; // Consume the name

    advance(); // Consume '='

    auto value = expression();

    if (current().type != TokenType::Semicolon) {
        throw std::runtime_error(
            "Expected ';' after assignment"
        );
    }

    advance();

    return std::make_unique<AssignStmt>(
        std::move(name),
        std::move(value)
    );
}

std::unique_ptr<Stmt> Parser::variableDeclaration() {

    advance(); // Consume 'var'

    VariableType type = typeName();

    advance();

    if (current().type != TokenType::Colon) {
        throw std::runtime_error(
            "Expected ':' after type name"
        );
    }

    advance();

    if (current().type != TokenType::Identifier) {
        throw std::runtime_error(
            "Expected variable name"
        );
    }

    std::string name = advance().value;

    if (current().type != TokenType::Equal) {
        throw std::runtime_error(
            "Expected '=' after variable name"
        );
    }

    advance();

    auto initializer = expression();

    if (current().type != TokenType::Semicolon) {
        throw std::runtime_error(
            "Expected ';' after variable declaration"
        );
    }

    advance();

    return std::make_unique<VariableDeclStmt>(
        type,
        std::move(name),
        std::move(initializer)
    );
}

std::unique_ptr<Expr> Parser::expression() {
    return logicalOr();
}

// '||' is the loosest binary operator: it binds less tightly than '&&',
// which in turn binds less tightly than any comparison.
std::unique_ptr<Expr> Parser::logicalOr() {

    auto left = logicalAnd();

    while (current().type == TokenType::OrOr) {
        BinaryOp op = toBinaryOp(advance().type);

        auto right = logicalAnd();

        left = std::make_unique<BinaryExpr>(
            std::move(left),
            op,
            std::move(right)
        );
    }

    return left;
}

std::unique_ptr<Expr> Parser::logicalAnd() {

    auto left = equality();

    while (current().type == TokenType::AndAnd) {
        BinaryOp op = toBinaryOp(advance().type);

        auto right = equality();

        left = std::make_unique<BinaryExpr>(
            std::move(left),
            op,
            std::move(right)
        );
    }

    return left;
}

std::unique_ptr<Expr> Parser::equality() {

    auto left = comparison();

    while (
        current().type == TokenType::EqualEqual ||
        current().type == TokenType::NotEqual
    ) {
        BinaryOp op = toBinaryOp(advance().type);

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
        BinaryOp op = toBinaryOp(advance().type);

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
        BinaryOp op = toBinaryOp(advance().type);

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
        BinaryOp op = toBinaryOp(advance().type);

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

    auto left = primary();

    // 'is' is a postfix operator, so it binds tighter than every binary
    // operator: 'a + b is(type: int)' parses as 'a + (b is(type: int))'.
    while (current().type == TokenType::Is) {
        left = isTypeCheck(std::move(left));
    }

    return left;
}

std::unique_ptr<Expr> Parser::isTypeCheck(
    std::unique_ptr<Expr> target
) {

    advance(); // Consume 'is'

    if (current().type != TokenType::LeftParen) {
        throw std::runtime_error(
            "Expected '(' after 'is'"
        );
    }

    advance();

    if (current().type != TokenType::TypeKeyword) {
        throw std::runtime_error(
            "Expected 'type' inside a type check"
        );
    }

    advance();

    if (current().type != TokenType::Colon) {
        throw std::runtime_error(
            "Expected ':' after 'type'"
        );
    }

    advance();

    VariableType expectedType = typeName();

    advance(); // Consume the type name

    if (current().type != TokenType::RightParen) {
        throw std::runtime_error(
            "Expected ')' to close a type check"
        );
    }

    advance();

    return std::make_unique<IsTypeExpr>(
        std::move(target),
        expectedType
    );
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

        return std::make_unique<VariableExpr>(
            std::move(name)
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
