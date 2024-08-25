#ifndef TOY_LANG_IRGEN_IRGEN_H
#define TOY_LANG_IRGEN_IRGEN_H

#include <map>
#include <ranges>

#include "ir/Function.h"
#include "ir/IRCompilationUnit.h"
#include "ir/Value.h"
#include "parser/AST.h"
#include "parser/ASTVisitor.h"

namespace irgen {

class NestedScope {
public:
  class ScopeGuard {
  public:
    ScopeGuard(NestedScope &Host) : Host(&Host) {}

    ScopeGuard(const ScopeGuard &) = delete;
    ScopeGuard &operator=(const ScopeGuard &) = delete;

    ScopeGuard(ScopeGuard &&Other) noexcept {
      std::swap(this->Host, Other.Host);
    }
    ScopeGuard &operator=(ScopeGuard &&Other) noexcept {
      std::swap(this->Host, Other.Host);
      return *this;
    }

    ~ScopeGuard() {
      if (Host != nullptr)
        Host->popScope();
    }

  private:
    NestedScope *Host = nullptr;
    ;
  };

  NestedScope() { ScopeStack.emplace_back(); }

  [[nodiscard]] ScopeGuard openNewScope() {
    ScopeStack.emplace_back();
    return ScopeGuard(*this);
  }

  void popScope() { ScopeStack.pop_back(); }

  Value *lookup(const std::string &Name) const {
    for (const auto &Iter : std::ranges::reverse_view(ScopeStack)) {
      if (auto Result = Iter.find(Name); Result != Iter.end())
        return Result->second;
    }

    return nullptr;
  }

  void update(std::string Name, Value *Value) {
    ScopeStack.back()[std::move(Name)] = Value;
  }

private:
  std::vector<std::map<std::string, Value *>> ScopeStack;
};

class IRGenerator : public ASTVisitor {
public:
  void visit(PrototypeAST &ProtoAST) override;
  void visit(FunctionAST &FnAST) override;
  void visit(CompilationUnit &Unit) override;

  IRCompilationUnit &getIR() { return IRUnit; }

private:
  Function *makeFunction(PrototypeAST &ProtoAST);

private:
  IRCompilationUnit IRUnit;
  NestedScope NS;
};

class FunctionVisitor : public ASTVisitor {
public:
  FunctionVisitor(IRCompilationUnit &IRUnit, NestedScope &NS, Function &Fn);

  void visit(BlockStmtAST &) override;
  void visit(IfStmtAST &) override;
  void visit(WhileStmtAST &) override;
  void visit(VarStmtAST &) override;
  void visit(ReturnStmtAST &) override;
  void visit(ExprStmtAST &) override;
  void visit(FunctionAST &FnAST) override;

private:
  IRCompilationUnit &IRUnit;
  NestedScope &NS;
  Function &Fn;
};

class ExprVisitor : public ASTVisitor {
public:
  ExprVisitor(IRCompilationUnit &IRUnit, NestedScope &NS, Function &Fn)
      : IRUnit(IRUnit),
        NS(NS),
        Fn(Fn) {}

  Value *getResult() { return Result; }

  void visit(NumberExprAST &Num) override;
  void visit(VariableExprAST &Var) override;
  void visit(UnaryExprAST &Unary) override;
  void visit(BinaryExprAST &Binary) override;
  void visit(CallExprAST &Call) override;

private:
  IRCompilationUnit &IRUnit;
  NestedScope &NS;
  Function &Fn;
  Value *Result = nullptr;
};

} // namespace irgen

#endif // !TOY_LANG_IRGEN_IRGEN_H
