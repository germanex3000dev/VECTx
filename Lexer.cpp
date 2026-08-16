#include "Lexer.hpp"
#include "Token.hpp"

#include <cctype>
#include <stdexcept>

Lexer::Lexer(const std::string& source)
   : source(source) {
}

char Lexer::current() const {
    if (position >= source.length()) {
        return '\0';
    }

    return source[position];
}

void Lexer::advance() {
    if (position < source.length()) {
        position++;
    }
}

Token Lexer::number() {

    std::string value;
    bool hasDecimal = false;

    while (std::isdigit(current())) {
        value += current();
        advance();
    }

    if (current() == '.') {

        hasDecimal = true;
        value += current();
        advance();

        while (std::isdigit(current())) {
            value += current();
            advance();
        }
    }

    if (hasDecimal) {
        return {
            TokenType::Float,
            value
        };
    }

    return {
        TokenType::Integer,
        value
    };
}

Token Lexer::string() {

    advance(); // Skip opening "

    std::string value;

    while (current() != '"' && current() != '\0') {
        value += current();
        advance();
    }

    if (current() == '\0') {
        throw std::runtime_error(
            "Unterminated string"
        );
    }

    advance(); // Skip closing "

    return {
        TokenType::String,
        value
    };
}

Token Lexer::identifier() {

    std::string value;

    while (
        std::isalnum(current()) ||
        current() == '_'
    ) {
        value += current();
        advance();
    }

    if (value == "true") {
        return {
            TokenType::True,
            value
        };
    }

    if (value == "false") {
        return {
            TokenType::False,
            value
        };
    }

    return {
        TokenType::Identifier,
        value
    };
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (position < source.length()) {
        char c = current();

        if (std::isspace(c)) {
            advance();
            continue;
        }

        if (std::isdigit(c)) {
            tokens.push_back(number());
            continue;
        }

        if (std::isalpha(c) || c == '_') {
          tokens.push_back(identifier());
          continue;
        }

        if (c == '"') {
          tokens.push_back(string());
          continue;
        }

        switch (c) {
            case '+':
                tokens.push_back({TokenType::Plus, "+"});
                break;
            
            case '-':
                tokens.push_back({TokenType::Minus, "-"});
                break;

            case '*':
                tokens.push_back({TokenType::Star, "*"});
                break;

            case '/':
                tokens.push_back({TokenType::Slash, "/"});
                break;

            case '(':
                tokens.push_back({TokenType::LeftParen, "("});
                break;

            case ')':
                tokens.push_back({TokenType::RightParen, ")"});
                break;

            case ',':
                tokens.push_back({TokenType::Comma, ","});
                break;

            case ';':
                tokens.push_back({TokenType::Semicolon, ";"});
                break;

            default:
                tokens.push_back({TokenType::Unknown, std::string(1, c)});
                break;

        }

        advance();
    }

    tokens.push_back({TokenType::EndOfFile, ""});

    return tokens;
}
