#include "target/aarch64/CodeGenerator.h"

#include <cassert>

#include "ir/AllocaInst.h"
#include "ir/BranchInst.h"
#include "ir/CallInst.h"

namespace aarch64 {

void CodeGenerator::visit(IRCompilationUnit &IRUnit) {
  for (auto &Fn : IRUnit) {
    if (Fn->getBlocks().empty()) {
      auto *Lbl = Unit.addExternalProcedure(Fn->getName());
      FnTable[Fn.get()] = Lbl;
      continue;
    }

    auto *Proc = Unit.makeNewProcedure(Fn->getName());
    FnTable[Fn.get()] = Proc->getEntryLabel();

    FunctionCG FnCG(*this, Unit, *Proc);
    Fn->accept(FnCG);
  }
}

Label *CodeGenerator::lookupFunctionEntry(Function *Fn) {
  return FnTable[Fn];
}

void FunctionCG::visit(Function &Fn) {
  Prologue = Proc.getPrologue();
  Proc.setInsertPoint(this->Prologue);
  for (auto &Param : Fn.getArgs())
    Param->accept(*this);

  for (auto &BB : Fn.getBlocks())
    BBTable[BB.get()] = Proc.makeNewLabel(std::string(BB->getName()));
  this->Epilogue = Proc.getEpilogue();

  for (auto &C : Fn.getConstants())
    C->accept(*this);

  for (auto &BB : Fn.getBlocks())
    BB->accept(*this);
}

void FunctionCG::visit(Parameter &Param) {
  // For arg[8], ..., they should be at old sp (before extend stack space),
  // sp + N (depends on the size of argument), ...
  assert(ArgCnt < 8 && "More than 8 arguments: unimplemented");

  // For arg[0], ..., arg[7], they should be mapped to physical register x0 - x7
  // ValueTable[&Param] = Unit.getPhysicsReg(ArgCnt++);
  auto SS = Proc.allocateStackSlot();
  Proc.emit<STR>(Unit.getPhysicsReg(ArgCnt), SS);
  ValueTable[&Param] = SS;

  ++ArgCnt;
}

void FunctionCG::visit(BasicBlock &BB) {
  // Emit all target code under this label.
  auto *TheLabel = BBTable[&BB];
  Proc.setInsertPoint(TheLabel);

  for (auto &Inst : BB)
    Inst->accept(*this);
}

void FunctionCG::visit(Constant &C) {
  // Make a new Constant Operand and store it in the ValueTable
  ValueTable[&C] = Proc.makeImm(C.getVal());
}

void FunctionCG::visit(AllocaInst &Inst) {
  // Assign a new StackSlot, store it in the ValueTable
  ValueTable[&Inst] = Proc.allocateStackSlot();
}

void FunctionCG::visit(StoreInst &Inst) {
  // Fetch the Operand from the ValueTable, emit a new STR inst
  auto *Ptr = ValueTable[Inst.getPtr()];
  auto *Val = ValueTable[Inst.getVal()];
  assert(Ptr->isMemory());
  assert(Val->isConstant() || Val->isRegister());
  Proc.emit<STR>(Val, Ptr);
}

void FunctionCG::visit(LoadInst &Inst) {
  // Make a new VirtualRegister. Emit a LDR inst. store it in the ValueTable.
  auto *Ptr = ValueTable[Inst.getPtr()];
  auto *Result = Proc.makeVirtReg();
  assert(Ptr->isMemory());
  Proc.emit<LDR>(Result, Ptr);
  ValueTable[&Inst] = Result;
}

void FunctionCG::visit(ArithmeticInst &Inst) {
  auto *LHS = ValueTable[Inst.getLHS()];
  auto *RHS = ValueTable[Inst.getRHS()];
  auto *Result = Proc.makeVirtReg();
  assert(LHS->isConstant() || LHS->isRegister());
  assert(RHS->isConstant() || RHS->isRegister());
  switch (Inst.getOpc()) {
  case ArithmeticInst::Opcode::Add: Proc.emit<ADD>(Result, LHS, RHS); break;
  case ArithmeticInst::Opcode::Sub: Proc.emit<SUB>(Result, LHS, RHS); break;
  case ArithmeticInst::Opcode::Mul: Proc.emit<MUL>(Result, LHS, RHS); break;
  }
  ValueTable[&Inst] = Result;
}

void FunctionCG::visit(JumpInst &Inst) {
  auto *Lbl = BBTable[Inst.getDest()];
  Proc.emit<B>(Lbl);
}

void FunctionCG::visit(CJumpInst &Inst) {
  auto *Cond = ValueTable[Inst.getCond()];
  auto *T = BBTable[Inst.getTrueBB()];
  auto *F = BBTable[Inst.getFalseBB()];
  Proc.emit<CBNZ>(Cond, T);
  Proc.emit<B>(F);
}

void FunctionCG::visit(CallInst &Inst) {
  auto &Args = Inst.getArguments();
  assert(Args.size() <= 8);
  for (size_t I = 0; I < Args.size(); ++I) {
    auto *Opr = ValueTable[Args[I]];
    assert(Opr->isConstant() || Opr->isRegister());
    Proc.emit<MOV>(Unit.getPhysicsReg(I), Opr);
  }
  Proc.emit<BL>(CG.lookupFunctionEntry(Inst.getCallee()));

  auto *Ret = Proc.makeVirtReg();
  Proc.emit<MOV>(Ret, Unit.getPhysicsReg(0));
  ValueTable[&Inst] = Ret;
}

void FunctionCG::visit(ReturnInst &Inst) {
  // Load or Move the value to the x0. Jump to the Epilogue.
  if (auto *Ret = Inst.getVal()) {
    auto *Opr = ValueTable[Ret];
    assert(Opr->isConstant() || Opr->isRegister());
    Proc.emit<MOV>(Unit.getPhysicsReg(0), Opr);
  }
  Proc.emit<B>(Epilogue);
}

} // namespace aarch64
