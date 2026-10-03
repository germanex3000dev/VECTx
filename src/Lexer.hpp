#pragma once

#include "Token.hpp"

#include <string>
#include <vector>

class Lexer{
public:
    explicit Lexer(const std::string& source);

    std::vector<Token> tokenize();

private:
    std::string source;
    std::size_t position = 0;

    char current() const;
    void advance();

    Token number();
    Token string();
    Token identifier();
};

