#include "parser/AST.h"

#include "parser/ASTVisitor.h"

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

void BlockStmtAST::accept(ASTVisitor &V) {
  V.visit(*this);
}

void IfStmtAST::accept(ASTVisitor &V) {
  V.visit(*this);
}

void WhileStmtAST::accept(ASTVisitor &V) {
  V.visit(*this);
}

void VarStmtAST::accept(ASTVisitor &V) {
  V.visit(*this);
}

void ReturnStmtAST::accept(ASTVisitor &V) {
  V.visit(*this);
}

void ExprStmtAST::accept(ASTVisitor &V) {
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
