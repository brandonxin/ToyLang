#pragma once

#include "fmt/format.h"

#include "parser/ASTVisitor.h"

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

  void visit(BlockStmtAST &S) override {
    print("BlockStmt");
    indent();
    for (const auto &Stmt : S.getStmts())
      Stmt->accept(*this);
    unindent();
  }

  void visit(IfStmtAST &S) override {
    print("IfStmt");
    indent();
    S.getCond().accept(*this);
    S.getThen().accept(*this);
    if (auto *Else = S.getElse(); Else != nullptr)
      Else->accept(*this);
    unindent();
  }

  void visit(WhileStmtAST &S) override {
    print("WhileStmt");
    indent();
    S.getCond().accept(*this);
    S.getBody().accept(*this);
    unindent();
  }

  void visit(VarStmtAST &E) override {
    print("VarStmt");
    indent();
    print(E.getVarName(), E.getVarType());
    if (auto *Init = E.getInit(); Init)
      E.getInit()->accept(*this);
    unindent();
  }

  void visit(ReturnStmtAST &S) override {
    print("ReturnStmt");
    auto *E = S.getExpr();
    if (!E)
      return;

    indent();
    E->accept(*this);
    unindent();
  }

  void visit(ExprStmtAST &S) override {
    print("ExprStmt");
    indent();
    S.getExpr().accept(*this);
    unindent();
  }

  void visit(PrototypeAST &P) override {
    std::string out;

    const auto &Params = P.getParams();
    if (Params.empty())
      fmt::format_to(std::back_inserter(out), "()");
    else {
      fmt::format_to(std::back_inserter(out), "({}", Params[0].second);
      for (size_t i = 1; i < Params.size(); ++i)
        fmt::format_to(std::back_inserter(out), ", {}", Params[i].second);
      fmt::format_to(std::back_inserter(out), ")");
    }
    fmt::format_to(std::back_inserter(out), " : {}", P.getReturnType());
    print("Prototype");
    indent();
    print(out);
    unindent();
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
