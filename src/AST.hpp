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

enum class VariableType {
    Integer,
    Float,
    Boolean,
    String,

    // Dynamic type: accepts any initializer and keeps whatever runtime
    // type the initializer produced.
    Any
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

struct Parameter {
    VariableType type;
    std::string name;
};

// def func name(<type>: <arg>, ...) -> <type>: <ret_glob> { ... }
//
// The signature's `-> <type>: <ret_glob>` declares the return variable. It
// lives in the function's own scope and is what a bare `return` yields, so
// `return ret_glob;` is redundant. `return <expr>;` yields <expr> instead, and
// that value must match the declared return type.
class FunctionStmt : public Stmt {
public:
  FunctionStmt(
      std::string name,
      std::vector<Parameter> parameters,
      VariableType returnType,
      std::string returnName,
      std::unique_ptr<BlockStmt> body
  )
    : name(std::move(name)),
      parameters(std::move(parameters)),
      returnType(returnType),
      returnName(std::move(returnName)),
      body(std::move(body)) {}

  std::string name;
  std::vector<Parameter> parameters;
  VariableType returnType;
  std::string returnName;
  std::unique_ptr<BlockStmt> body;
};

// return [<expr>];
//
// Without a value it yields the declared return variable. A function that
// falls off its end yields that variable too.
class ReturnStmt : public Stmt {
public:
  explicit ReturnStmt(std::unique_ptr<Expr> value)
    : value(std::move(value)) {}

  std::unique_ptr<Expr> value;
};

// name = value;
//
// Reassigns an existing variable. 'any' variables may take a different type
// than they had before; a concretely typed variable may not.
class AssignStmt : public Stmt {
public:
  AssignStmt(std::string name, std::unique_ptr<Expr> value)
    : name(std::move(name)), value(std::move(value)) {}

  std::string name;
  std::unique_ptr<Expr> value;
};

class Program {
public:
  std::vector<std::unique_ptr<Stmt>> statements;

  // Collected separately from the statements so they can all be registered
  // before anything runs, which is what lets a function call another one
  // declared later.
  std::vector<std::unique_ptr<FunctionStmt>> functions;
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
    GreaterEqual,

    // Lowest precedence: && binds tighter than ||, both looser than the
    // comparisons above.
    LogicalAnd,
    LogicalOr
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
