#include "irgen/IRGenerator.h"
#include "parser/AST.h"

namespace irgen {

Function *IRGenerator::makeFunction(PrototypeAST &ProtoAST) {
  std::vector<std::string> ParamsName;
  for (const auto &[Name, Type] : ProtoAST.getParams())
    ParamsName.push_back(Name);

  return IRUnit.makeNewFunction(ProtoAST.getName(), std::move(ParamsName));
}

void IRGenerator::visit(PrototypeAST &ProtoAST) {
  auto *Fn = IRUnit.lookupFunction(ProtoAST.getName());
  if (!Fn)
    makeFunction(ProtoAST);
}

void IRGenerator::visit(FunctionAST &FnAST) {
  auto *Fn = IRUnit.lookupFunction(FnAST.getProto().getName());
  if (!Fn)
    Fn = makeFunction(FnAST.getProto());

  FunctionVisitor FnVisitor(IRUnit, NS, *Fn);
  Fn->setInsertPoint(Fn->createEntryBlock());
  FnAST.accept(FnVisitor);
}

void IRGenerator::visit(CompilationUnit &Unit) {
  for (const auto &Decls : Unit.getDecls()) {
    Decls->accept(*this);
  }
}

void FunctionVisitor::visit(BlockStmtAST &Block) {
  // Open a new scope
  auto ScopeGuard = NS.openNewScope();
  for (const auto &Stmt : Block.getStmts())
    Stmt->accept(*this);
}

void FunctionVisitor::visit(IfStmtAST &If) {
  ExprVisitor CondVisitor(IRUnit, NS, Fn);
  If.getCond().accept(CondVisitor);
  auto *Cond = CondVisitor.getResult();

  auto *ThenBB = Fn.makeNewBlock();
  auto *ElseBB = Fn.makeNewBlock();
  auto *FinalBB = Fn.makeNewBlock();
  if (If.getElse()) {
    Fn.appendCJump(Cond, ThenBB, ElseBB);
  } else {
    Fn.appendCJump(Cond, ThenBB, FinalBB);
  }

  Fn.setInsertPoint(ThenBB);
  If.getThen().accept(*this);
  Fn.appendJump(FinalBB);

  if (auto *Else = If.getElse()) {
    Fn.setInsertPoint(ElseBB);
    Else->accept(*this);
    Fn.appendJump(FinalBB);
  }

  Fn.setInsertPoint(FinalBB);
}

void FunctionVisitor::visit(WhileStmtAST &While) {
  auto *CondBB = Fn.makeNewBlock();
  auto *LoopBB = Fn.makeNewBlock();
  auto *FinalBB = Fn.makeNewBlock();

  Fn.appendJump(CondBB);
  Fn.setInsertPoint(CondBB);

  ExprVisitor CondVisitor(IRUnit, NS, Fn);
  While.getCond().accept(CondVisitor);
  Fn.appendCJump(CondVisitor.getResult(), LoopBB, FinalBB);

  Fn.setInsertPoint(LoopBB);
  While.getBody().accept(*this);
  Fn.appendJump(CondBB);

  Fn.setInsertPoint(FinalBB);
}

void FunctionVisitor::visit(VarStmtAST &Var) {
  auto *Alloca = Fn.apendAlloca();
  NS.update(std::string(Var.getVarName()), Alloca);

  auto *Expr = Var.getInit();
  if (!Expr)
    return;

  ExprVisitor V(IRUnit, NS, Fn);
  Expr->accept(V);
  Fn.appendStore(Alloca, V.getResult());
}

void FunctionVisitor::visit(ReturnStmtAST &Return) {
  if (auto *Expr = Return.getExpr()) {
    ExprVisitor V(IRUnit, NS, Fn);
    Return.accept(V);
    Fn.appendReturn(V.getResult());
  } else {
    Fn.appendReturn();
  }
}

void FunctionVisitor::visit(ExprStmtAST &ExprStmt) {
  ExprVisitor V(IRUnit, NS, Fn);
  ExprStmt.accept(V);
}

void ExprVisitor::visit(NumberExprAST &NumAST) {
  Result = Fn.makeConstant(NumAST.getVal());
}

void ExprVisitor::visit(VariableExprAST &Var) {
  Result = NS.lookup(Var.getName());
}

void ExprVisitor::visit(UnaryExprAST &Unary) {
  ExprVisitor V(IRUnit, NS, Fn);
  Unary.getOperand().accept(V);
  auto *Operand = V.getResult();
  switch (Unary.getOpcode()) {
  case '-': Result = Fn.appendSub(Fn.makeConstant(0), Operand); break;
  default: assert(false && "Unknown unary operator");
  }
}

void ExprVisitor::visit(BinaryExprAST &Binary) {
  ExprVisitor LHSVisitor(IRUnit, NS, Fn);
  ExprVisitor RHSVisitor(IRUnit, NS, Fn);
  Binary.getLHS().accept(LHSVisitor);
  Binary.getRHS().accept(RHSVisitor);
  auto *LHS = LHSVisitor.getResult();
  auto *RHS = RHSVisitor.getResult();

  if (Binary.getOpcode() != '=') {
    if (LHS->isLValue())
      LHS = Fn.appendLoad(LHS);
  }
  if (RHS->isLValue())
    RHS = Fn.appendLoad(RHS);

  switch (Binary.getOpcode()) {
  case '+': Result = Fn.appendAdd(LHS, RHS); break;
  case '-': Result = Fn.appendSub(LHS, RHS); break;
  case '*': Result = Fn.appendMul(LHS, RHS); break;
  case '=': // What is LHS, a loaded Value or an Alloca?
    Fn.appendStore(LHS, RHS);
    Result = RHS;
    break;
  default: assert(false && "Unknown binary operator");
  }
}

void ExprVisitor::visit(CallExprAST &Call) {
  auto *Callee = IRUnit.lookupFunction(Call.getCallee());
  std::vector<Value *> Args;
  for (const auto &Arg : Call.getArgs()) {
    ExprVisitor V(IRUnit, NS, Fn);
    Arg->accept(V);
    auto *Result = V.getResult();
    if (Result->isLValue())
      Result = Fn.appendLoad(Result);
    Args.push_back(Result);
  }
  Fn.appendCall(Callee, std::move(Args));
}

} // namespace irgen
