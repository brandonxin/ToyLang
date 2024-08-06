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

/// IfExprAST - Expression class for if/then/else.
class IfExprAST : public ExprAST {
  std::unique_ptr<ExprAST> Cond, Then, Else;

public:
  IfExprAST(std::unique_ptr<ExprAST> Cond, std::unique_ptr<ExprAST> Then,
            std::unique_ptr<ExprAST> Else)
      : Cond(std::move(Cond)),
        Then(std::move(Then)),
        Else(std::move(Else)) {}

  // Value *codegen() override;

  void accept(ASTVisitor &V) override;

  ExprAST &getCond() const { return *Cond; }
  ExprAST &getThen() const { return *Then; }
  ExprAST *getElse() const { return Else.get(); }
};

/// ForExprAST - Expression class for for/in.
class ForExprAST : public ExprAST {
  std::string VarName;
  std::unique_ptr<ExprAST> Start, End, Step, Body;

public:
  ForExprAST(const std::string &VarName, std::unique_ptr<ExprAST> Start,
             std::unique_ptr<ExprAST> End, std::unique_ptr<ExprAST> Step,
             std::unique_ptr<ExprAST> Body)
      : VarName(VarName),
        Start(std::move(Start)),
        End(std::move(End)),
        Step(std::move(Step)),
        Body(std::move(Body)) {}

  // Value *codegen() override;

  void accept(ASTVisitor &V) override;

  const std::string &getVarName() const { return VarName; }
  ExprAST &getStart() const { return *Start; }
  ExprAST &getEnd() const { return *End; }
  ExprAST &getStep() const { return *Step; }
  ExprAST &getBody() const { return *Body; }
};

/// VarExprAST - Expression class for var/in
class VarExprAST : public ExprAST {
  std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> VarNames;
  std::unique_ptr<ExprAST> Body;

public:
  VarExprAST(
      std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> VarNames,
      std::unique_ptr<ExprAST> Body)
      : VarNames(std::move(VarNames)),
        Body(std::move(Body)) {}

  // Value *codegen() override;

  void accept(ASTVisitor &V) override;

  const std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> &
  getVarNames() const {
    return VarNames;
  }
  ExprAST &getBody() const { return *Body; }
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes), as well as if it is an operator.
class PrototypeAST {
  std::string Name;
  std::vector<std::string> Args;
  bool IsOperator;
  unsigned Precedence; // Precedence if a binary op.

public:
  PrototypeAST(const std::string &Name, std::vector<std::string> Args,
               bool IsOperator = false, unsigned Prec = 0)
      : Name(Name),
        Args(std::move(Args)),
        IsOperator(IsOperator),
        Precedence(Prec) {}

  // Function *codegen();

  void accept(ASTVisitor &V);

  const auto &getName() const { return Name; }
  const auto &getArgs() const { return Args; }

  bool isUnaryOp() const { return IsOperator && Args.size() == 1; }
  bool isBinaryOp() const { return IsOperator && Args.size() == 2; }

  char getOperatorName() const {
    assert(isUnaryOp() || isBinaryOp());
    return Name[Name.size() - 1];
  }

  unsigned getBinaryPrecedence() const { return Precedence; }
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST {
  std::unique_ptr<PrototypeAST> Proto;
  std::unique_ptr<ExprAST> Body;

public:
  FunctionAST(std::unique_ptr<PrototypeAST> Proto,
              std::unique_ptr<ExprAST> Body)
      : Proto(std::move(Proto)),
        Body(std::move(Body)) {}

  // Function *codegen();

  void accept(ASTVisitor &V);

  PrototypeAST &getProto() const { return *Proto; }
  ExprAST &getBody() const { return *Body; }
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
