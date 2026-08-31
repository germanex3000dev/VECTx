#pragma once

#include <string>

enum class TokenType {
    Integer,
    Float,
    String,

    True,
    False,

    Identifier,

    Plus,
    Minus,
    Star,
    Slash,

    EqualEqual,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,

    LeftParen,
    RightParen,
    Comma,
    Semicolon,

    EndOfFile,
    Unknown
};

struct Token {
    TokenType type;
    std::string value;
};
