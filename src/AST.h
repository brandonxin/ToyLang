#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

//===----------------------------------------------------------------------===//
// Abstract Syntax Tree (aka Parse Tree)
//===----------------------------------------------------------------------===//

class ASTVisitor;

/// ExprAST - Base class for all expression nodes.
class ExprAST {
public:
  virtual ~ExprAST() = default;

  //   virtual Value *codegen() = 0;

  virtual void accept(ASTVisitor &V) = 0;
};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
  int64_t Val;

public:
  NumberExprAST(int64_t Val) : Val(Val) {}

  // Value *codegen() override;

  void accept(ASTVisitor &V) override;

  int64_t getVal() const { return Val; }
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
  std::string Name;

public:
  VariableExprAST(const std::string &Name) : Name(Name) {}

  // Value *codegen() override;

  void accept(ASTVisitor &V) override;

  const std::string &getName() const { return Name; }
};

/// UnaryExprAST - Expression class for a unary operator.
class UnaryExprAST : public ExprAST {
  char Opcode;
  std::unique_ptr<ExprAST> Operand;

public:
  UnaryExprAST(char Opcode, std::unique_ptr<ExprAST> Operand)
      : Opcode(Opcode),
        Operand(std::move(Operand)) {}

  // Value *codegen() override;

  void accept(ASTVisitor &V) override;

  char getOpcode() const { return Opcode; }
  ExprAST &getOperand() const { return *Operand; }
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
  char Op;
  std::unique_ptr<ExprAST> LHS, RHS;

public:
  BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS,
                std::unique_ptr<ExprAST> RHS)
      : Op(Op),
        LHS(std::move(LHS)),
        RHS(std::move(RHS)) {}

  // Value *codegen() override;
  void accept(ASTVisitor &V) override;

  char getOpcode() const { return Op; }
  ExprAST &getLHS() const { return *LHS; }
  ExprAST &getRHS() const { return *RHS; }
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
  std::string Callee;
  std::vector<std::unique_ptr<ExprAST>> Args;

public:
  CallExprAST(const std::string &Callee,
              std::vector<std::unique_ptr<ExprAST>> Args)
      : Callee(Callee),
        Args(std::move(Args)) {}

  // Value *codegen() override;

  void accept(ASTVisitor &V) override;

  const std::string &getCallee() const { return Callee; }
  const std::vector<std::unique_ptr<ExprAST>> &getArgs() const { return Args; }
};

class StmtAST {
public:
  virtual ~StmtAST() = default;

  virtual void accept(ASTVisitor &V) = 0;
};

class BlockStmtAST : public StmtAST {
public:
  BlockStmtAST(std::vector<std::unique_ptr<StmtAST>> Stmts)
      : Stmts(std::move(Stmts)) {}

  void accept(ASTVisitor &V) override;

  const auto &getStmts() const { return Stmts; }

private:
  std::vector<std::unique_ptr<StmtAST>> Stmts;
};

/// IfExprAST - Expression class for if/then/else.
class IfStmtAST : public StmtAST {
  std::unique_ptr<ExprAST> Cond;
  std::unique_ptr<StmtAST> Then, Else;

public:
  IfStmtAST(std::unique_ptr<ExprAST> Cond, std::unique_ptr<StmtAST> Then)
      : Cond(std::move(Cond)),
        Then(std::move(Then)),
        Else(nullptr) {}

  IfStmtAST(std::unique_ptr<ExprAST> Cond, std::unique_ptr<StmtAST> Then,
            std::unique_ptr<StmtAST> Else)
      : Cond(std::move(Cond)),
        Then(std::move(Then)),
        Else(std::move(Else)) {}

  // Value *codegen() override;

  void accept(ASTVisitor &V) override;

  ExprAST &getCond() const { return *Cond; }
  StmtAST &getThen() const { return *Then; }
  StmtAST *getElse() const { return Else.get(); }
};

class WhileStmtAST : public StmtAST {
public:
  WhileStmtAST(std::unique_ptr<ExprAST> Cond, std::unique_ptr<StmtAST> Body)
      : Cond(std::move(Cond)),
        Body(std::move(Body)) {}

  void accept(ASTVisitor &V) override;

  ExprAST &getCond() const { return *Cond; }
  StmtAST &getBody() const { return *Body; }

private:
  std::unique_ptr<ExprAST> Cond;
  std::unique_ptr<StmtAST> Body;
};

/// VarExprAST - Expression class for var/in
class VarStmtAST : public StmtAST {
  std::string VarName;
  std::string VarType;
  std::unique_ptr<ExprAST> Init;

public:
  VarStmtAST(std::string VarName, std::string VarType,
             std::unique_ptr<ExprAST> Init)
      : VarName(std::move(VarName)),
        VarType(std::move(VarType)),
        Init(std::move(Init)) {}

  // Value *codegen() override;

  void accept(ASTVisitor &V) override;

  std::string_view getVarName() const { return VarName; }
  std::string_view getVarType() const { return VarType; }
  ExprAST *getInit() const { return Init.get(); }
};

class ReturnStmtAST : public StmtAST {
public:
  ReturnStmtAST() : Expr(nullptr) {}
  ReturnStmtAST(std::unique_ptr<ExprAST> Expr) : Expr(std::move(Expr)) {}

  void accept(ASTVisitor &V) override;

  ExprAST *getExpr() const { return Expr.get(); }

private:
  std::unique_ptr<ExprAST> Expr;
};

class ExprStmtAST : public StmtAST {
public:
  ExprStmtAST(std::unique_ptr<ExprAST> Expr) : Expr(std::move(Expr)) {}

  void accept(ASTVisitor &V) override;

  ExprAST &getExpr() const { return *Expr; }

private:
  std::unique_ptr<ExprAST> Expr;
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes), as well as if it is an operator.
class PrototypeAST {
  std::string ReturnType;
  std::string Name;
  std::vector<std::pair<std::string, std::string>> Params;

public:
  PrototypeAST(std::string Name,
               std::vector<std::pair<std::string, std::string>> Params)
      : ReturnType("void"),
        Name(std::move(Name)),
        Params(std::move(Params)) {}
  PrototypeAST(std::string ReturnType, std::string Name,
               std::vector<std::pair<std::string, std::string>> Params)
      : ReturnType(std::move(ReturnType)),
        Name(std::move(Name)),
        Params(std::move(Params)) {}

  // Function *codegen();

  void accept(ASTVisitor &V);

  const auto &getReturnType() const { return ReturnType; }
  const auto &getName() const { return Name; }
  const auto &getParams() const { return Params; }
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST {
  std::unique_ptr<PrototypeAST> Proto;
  std::unique_ptr<StmtAST> Body;

public:
  FunctionAST(std::unique_ptr<PrototypeAST> Proto,
              std::unique_ptr<StmtAST> Body)
      : Proto(std::move(Proto)),
        Body(std::move(Body)) {}

  // Function *codegen();

  void accept(ASTVisitor &V);

  PrototypeAST &getProto() const { return *Proto; }
  StmtAST &getBody() const { return *Body; }
};

class CompilationUnit {
public:
  CompilationUnit() = default;
  CompilationUnit(std::vector<std::unique_ptr<PrototypeAST>> Protos,
                  std::vector<std::unique_ptr<FunctionAST>> Funcs)
      : Protos(std::move(Protos)),
        Funcs(std::move(Funcs)) {}

  void addPrototype(std::unique_ptr<PrototypeAST> &&Proto) {
    Protos.emplace_back(std::move(Proto));
  }

  void addFunction(std::unique_ptr<FunctionAST> &&Func) {
    Funcs.emplace_back(std::move(Func));
  }

  const auto &getProtos() const { return Protos; }
  const auto &getFuncs() const { return Funcs; }

  void accept(ASTVisitor &V);

private:
  std::vector<std::unique_ptr<PrototypeAST>> Protos;
  std::vector<std::unique_ptr<FunctionAST>> Funcs;
};
