#pragma once

#include <string>

enum class TokenType {
    Integer,
    Float,
    String,

    True,
    False,

    Var,
    IntType,
    FloatType,
    StringType,
    BoolType,
    AnyType,

    Is,
    TypeKeyword,

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

    Equal,
    Colon,

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
