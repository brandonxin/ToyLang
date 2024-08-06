#include "AST.h"

#include "ASTVisitor.h"

void NumberExprAST::accept(ASTVisitor &V) {
  V.visit(*this);
}

void VariableExprAST::accept(ASTVisitor &V) {
  V.visit(*this);
}

void UnaryExprAST::accept(ASTVisitor &V) {
  V.visit(*this);
}

void BinaryExprAST::accept(ASTVisitor &V) {
  V.visit(*this);
}

void CallExprAST::accept(ASTVisitor &V) {
  V.visit(*this);
}

void IfExprAST::accept(ASTVisitor &V) {
  V.visit(*this);
}

void ForExprAST::accept(ASTVisitor &V) {
  V.visit(*this);
}

void VarExprAST::accept(ASTVisitor &V) {
  V.visit(*this);
}

void PrototypeAST::accept(ASTVisitor &V) {
  V.visit(*this);
}

void FunctionAST::accept(ASTVisitor &V) {
  V.visit(*this);
}

void CompilationUnit::accept(ASTVisitor &V) {
  V.visit(*this);
}
