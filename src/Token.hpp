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

    Def,
    Func,
    Return,

    Arrow,
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

    AndAnd,
    OrOr,

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
