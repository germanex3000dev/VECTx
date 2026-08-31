#pragma once

#include <memory>
#include <string>
#include <vector>

class Expr {
public:
    virtual ~Expr() = default;
};

class Stmt {
public:
  virtual ~Stmt() = default;
};

class ExpressionStmt : public Stmt {
public:
  explicit ExpressionStmt(std::unique_ptr<Expr> expression)
    : expression(std::move(expression)) {}

  std::unique_ptr<Expr> expression;
};

class Program {
public:
  std::vector<std::unique_ptr<Stmt>> statements;
};

class NumberExpr : public Expr {
public:
    explicit NumberExpr(int value)
        : value(value) {}
    
    int value;
};

class BinaryExpr : public Expr {
public:
    BinaryExpr(
        std::unique_ptr<Expr> left,
        std::string op,
        std::unique_ptr<Expr> right
    )
        : left(std::move(left)),
          op(std::move(op)),
          right(std::move(right)) {}
    std::unique_ptr<Expr> left;
    std::string op;
    std::unique_ptr<Expr> right;
};

class CallExpr : public Expr {
public: 
    CallExpr(
        std::string name,
        std::vector<std::unique_ptr<Expr>> arguments
    )
        : name(std::move(name)),
          arguments(std::move(arguments)) {}

    std::string name;
    std::vector<std::unique_ptr<Expr>> arguments;
};

class FloatExpr : public Expr {
public:
    explicit FloatExpr(double value)
        : value(value) {}

    double value;
};

class StringExpr : public Expr {
public:
  explicit StringExpr(std::string value)
    : value(std::move(value)) {}

  std::string value;
};

class BooleanExpr : public Expr {
public:
  explicit BooleanExpr(bool value)
    : value(value) {}

  bool value;
};
