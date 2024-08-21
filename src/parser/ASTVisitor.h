#pragma once

#include "parser/AST.h"

class ASTVisitor {
public:
  virtual ~ASTVisitor() = default;
  virtual void visit(NumberExprAST &) {}
  virtual void visit(VariableExprAST &) {}
  virtual void visit(UnaryExprAST &) {}
  virtual void visit(BinaryExprAST &) {}
  virtual void visit(CallExprAST &) {}
  virtual void visit(BlockStmtAST &) {}
  virtual void visit(IfStmtAST &) {}
  virtual void visit(WhileStmtAST &) {}
  virtual void visit(VarStmtAST &) {}
  virtual void visit(ReturnStmtAST &) {}
  virtual void visit(ExprStmtAST &) {}
  virtual void visit(PrototypeAST &) {}
  virtual void visit(FunctionAST &) {}
  virtual void visit(CompilationUnit &) {}
};
