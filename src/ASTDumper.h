#pragma once

#include "fmt/format.h"

#include "ASTVisitor.h"

class ASTDumper : public ASTVisitor {
public:
  ASTDumper(std::FILE *OS, CompilationUnit &U) : OS(OS) { U.accept(*this); }

  void visit(NumberExprAST &E) override { print("Number", E.getVal()); }

  void visit(VariableExprAST &E) override { print("Variable", E.getName()); }

  void visit(UnaryExprAST &E) override {
    print("UnaryExpr", E.getOpcode());
    indent();
    E.getOperand().accept(*this);
    unindent();
  }

  void visit(BinaryExprAST &E) override {
    print("BinaryExpr", E.getOpcode());
    indent();
    E.getLHS().accept(*this);
    E.getRHS().accept(*this);
    unindent();
  }

  void visit(CallExprAST &E) override {
    print("CallExpr", E.getCallee());
    indent();
    for (const auto &Arg : E.getArgs())
      Arg->accept(*this);
    unindent();
  }

  void visit(IfExprAST &E) override {
    print("IfExpr");
    indent();
    E.getCond().accept(*this);
    E.getThen().accept(*this);
    if (auto *Else = E.getElse(); Else != nullptr)
      Else->accept(*this);
    unindent();
  }

  void visit(ForExprAST &E) override {
    print("ForExpr");
    indent();
    E.getStart().accept(*this);
    E.getEnd().accept(*this);
    E.getStep().accept(*this);
    E.getBody().accept(*this);
    unindent();
  }

  void visit(VarExprAST &E) override {
    print("VarExpr");
    indent();
    for (const auto &Var : E.getVarNames()) {
      print("Variable", Var.first);
      indent();
      if (Var.second)
        Var.second->accept(*this);
      unindent();
    }
    unindent();
  }

  void visit(PrototypeAST &P) override {
    print("Prototype",
          fmt::format("{}({})", P.getName(),
                      fmt::join(P.getArgs().begin(), P.getArgs().end(), ",")));
  }

  void visit(FunctionAST &Fn) override {
    print("Function");
    indent();
    Fn.getProto().accept(*this);
    Fn.getBody().accept(*this);
    unindent();
  }

  void visit(CompilationUnit &U) override {
    for (const auto &P : U.getProtos())
      P->accept(*this);
    for (const auto &Fn : U.getFuncs())
      Fn->accept(*this);
  }

protected:
  void indent() { ++Indent; }

  void unindent() { --Indent; }

  template <typename... T>
  void print(std::string_view name, T &&...Value) {
    printIndent();
    fmt::println(OS, "{} ({})", name, std::forward<T>(Value)...);
  }

  void print(std::string_view name) {
    printIndent();
    fmt::println(OS, "{}", name);
  }

  void printIndent() {
    for (int i = 0; i < Indent; ++i)
      fmt::print(OS, "    ");
  }

private:
  std::FILE *OS;

  int Indent = 0;
};
