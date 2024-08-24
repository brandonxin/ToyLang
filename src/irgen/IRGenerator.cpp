#include "irgen/IRGenerator.h"
#include "ir/Alloca.h"
#include "ir/BranchInst.h"
#include "ir/CallInst.h"
#include "ir/Instruction.h"
#include "parser/AST.h"

namespace irgen {

Function *IRGenerator::makeFunction(PrototypeAST &ProtoAST) {
  return IRUnit.makeNewFunction(ProtoAST.getName(),
                                ProtoAST.getParams().size());
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

FunctionVisitor::FunctionVisitor(IRCompilationUnit &IRUnit, NestedScope &NS,
                                 Function &Fn)
    : IRUnit(IRUnit),
      NS(NS),
      Fn(Fn) {}

void FunctionVisitor::visit(IfStmtAST &If) {
  ExprVisitor CondVisitor(IRUnit, NS, Fn);
  If.getCond().accept(CondVisitor);
  auto *Cond = CondVisitor.getResult();

  auto *ThenBB = Fn.makeNewBlock();
  auto *ElseBB = Fn.makeNewBlock();
  auto *FinalBB = Fn.makeNewBlock();
  if (If.getElse()) {
    Fn.emit<CJumpInst>(Cond, ThenBB, ElseBB);
  } else {
    Fn.emit<CJumpInst>(Cond, ThenBB, FinalBB);
  }

  Fn.setInsertPoint(ThenBB);
  If.getThen().accept(*this);
  Fn.emit<JumpInst>(FinalBB);

  if (auto *Else = If.getElse()) {
    Fn.setInsertPoint(ElseBB);
    Else->accept(*this);
    Fn.emit<JumpInst>(FinalBB);
  }

  Fn.setInsertPoint(FinalBB);
}

void FunctionVisitor::visit(WhileStmtAST &While) {
  auto *CondBB = Fn.makeNewBlock();
  auto *LoopBB = Fn.makeNewBlock();
  auto *FinalBB = Fn.makeNewBlock();

  Fn.emit<JumpInst>(CondBB);
  Fn.setInsertPoint(CondBB);

  ExprVisitor CondVisitor(IRUnit, NS, Fn);
  While.getCond().accept(CondVisitor);
  Fn.emit<CJumpInst>(CondVisitor.getResult(), LoopBB, FinalBB);

  Fn.setInsertPoint(LoopBB);
  While.getBody().accept(*this);
  Fn.emit<JumpInst>(CondBB);

  Fn.setInsertPoint(FinalBB);
}

void FunctionVisitor::visit(VarStmtAST &Var) {
  Instruction *Alloca = Fn.emit<AllocaInst>();
  NS.update(std::string(Var.getVarName()), Alloca);

  auto *Expr = Var.getInit();
  if (!Expr)
    return;

  ExprVisitor V(IRUnit, NS, Fn);
  Expr->accept(V);
  Fn.emit<StoreInst>(Alloca, V.getResult());
}

void FunctionVisitor::visit(ReturnStmtAST &Return) {
  if (auto *Expr = Return.getExpr()) {
    ExprVisitor V(IRUnit, NS, Fn);
    Return.accept(V);
    Fn.emit<ReturnInst>(V.getResult());
  } else {
    Fn.emit<ReturnInst>();
  }
}

void FunctionVisitor::visit(ExprStmtAST &ExprStmt) {
  ExprVisitor V(IRUnit, NS, Fn);
  ExprStmt.accept(V);
}

void FunctionVisitor::visit(FunctionAST &FnAST) {
  const auto &Params = FnAST.getProto().getParams();
  const auto &ParamsValue = Fn.getParams();
  assert(Params.size() == ParamsValue.size());

  auto Guard = NS.openNewScope();
  for (size_t I = 0; I < Params.size(); ++I)
    NS.update(Params[I].first, ParamsValue[I].get());

  FnAST.getBody().accept(*this);
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
  case '-':
    Result = Fn.emit<ArithmeticInst>(ArithmeticInst::Opcode::Sub,
                                     Fn.makeConstant(0), Operand);
    break;
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
      LHS = Fn.emit<LoadInst>(LHS);
  }
  if (RHS->isLValue())
    RHS = Fn.emit<LoadInst>(RHS);

  switch (Binary.getOpcode()) {
  case '+':
    Result = Fn.emit<ArithmeticInst>(ArithmeticInst::Opcode::Add, LHS, RHS);
    break;
  case '-':
    Result = Fn.emit<ArithmeticInst>(ArithmeticInst::Opcode::Sub, LHS, RHS);
    break;
  case '*':
    Result = Fn.emit<ArithmeticInst>(ArithmeticInst::Opcode::Mul, LHS, RHS);
    break;
  case '=':
    Fn.emit<StoreInst>(LHS, RHS);
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
      Result = Fn.emit<LoadInst>(Result);
    Args.push_back(Result);
  }
  Fn.emit<CallInst>(Callee, std::move(Args));
}

} // namespace irgen
