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

// A braced group of statements. Also the scope of any variable declared
// inside it, which is discarded when the block ends.
class BlockStmt : public Stmt {
public:
  explicit BlockStmt(
      std::vector<std::unique_ptr<Stmt>> statements
  )
    : statements(std::move(statements)) {}

  std::vector<std::unique_ptr<Stmt>> statements;
};

// if (condition) thenBranch [else elseBranch]
//
// elseBranch is null when there is no else clause. An `else if` chain is
// represented by nesting another IfStmt in the elseBranch.
class IfStmt : public Stmt {
public:
  IfStmt(
      std::unique_ptr<Expr> condition,
      std::unique_ptr<Stmt> thenBranch,
      std::unique_ptr<Stmt> elseBranch
  )
    : condition(std::move(condition)),
      thenBranch(std::move(thenBranch)),
      elseBranch(std::move(elseBranch)) {}

  std::unique_ptr<Expr> condition;
  std::unique_ptr<Stmt> thenBranch;
  std::unique_ptr<Stmt> elseBranch;
};

enum class VariableType {
    Integer,
    Float,
    Boolean,
    String,

    // Dynamic type: accepts any initializer and keeps whatever runtime
    // type the initializer produced.
    Any
};

class VariableDeclStmt : public Stmt {
public:
  VariableDeclStmt(
      VariableType type,
      std::string name,
      std::unique_ptr<Expr> initializer
  )
    : type(type),
      name(std::move(name)),
      initializer(std::move(initializer)) {}

  VariableType type;
  std::string name;
  std::unique_ptr<Expr> initializer;
};

class NumberExpr : public Expr {
public:
    explicit NumberExpr(int value)
        : value(value) {}
    
    int value;
};

enum class BinaryOp {
    Add,
    Subtract,
    Multiply,
    Divide,
    Equal,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual
};

class BinaryExpr : public Expr {
public:
    BinaryExpr(
        std::unique_ptr<Expr> left,
        BinaryOp op,
        std::unique_ptr<Expr> right
    )
        : left(std::move(left)),
          op(op),
          right(std::move(right)) {}
    std::unique_ptr<Expr> left;
    BinaryOp op;
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

class VariableExpr : public Expr {
public:
    explicit VariableExpr(std::string name)
        : name(std::move(name)) {}

    std::string name;
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

class IsTypeExpr : public Expr {
public:
  IsTypeExpr(
      std::unique_ptr<Expr> target,
      VariableType expectedType
  )
    : target(std::move(target)),
      expectedType(expectedType) {}

  std::unique_ptr<Expr> target;
  VariableType expectedType;
};
