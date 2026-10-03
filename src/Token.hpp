#pragma once

#include <string>

enum class TokenType {
    Integer,
    Float,
    String,

    True,
    False,

    Var,
    If,
    Else,
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
    LeftBrace,
    RightBrace,
    Comma,
    Semicolon,

    EndOfFile,
    Unknown
};

struct Token {
    TokenType type;
    std::string value;
};
