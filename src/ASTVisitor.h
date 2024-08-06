#pragma once

#include "AST.h"

class ASTVisitor {
public:
  virtual ~ASTVisitor() = default;
  virtual void visit(NumberExprAST &) {}
  virtual void visit(VariableExprAST &) {}
  virtual void visit(UnaryExprAST &) {}
  virtual void visit(BinaryExprAST &) {}
  virtual void visit(CallExprAST &) {}
  virtual void visit(IfExprAST &) {}
  virtual void visit(ForExprAST &) {}
  virtual void visit(VarExprAST &) {}
  virtual void visit(PrototypeAST &) {}
  virtual void visit(FunctionAST &) {}
  virtual void visit(CompilationUnit &) {}
};
