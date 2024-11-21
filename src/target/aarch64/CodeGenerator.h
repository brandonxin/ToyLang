#ifndef TOY_LANG_TARGET_AARCH64_CODE_GENERATOR_H
#define TOY_LANG_TARGET_AARCH64_CODE_GENERATOR_H

#include <unordered_map>

#include "ir/IRCompilationUnit.h"
#include "ir/IRVisitor.h"
#include "target/aarch64/Assembly.h"
#include "target/aarch64/AssemblyUnit.h"

namespace aarch64 {

class CodeGenerator : public IRVisitor {
  AssemblyUnit &Unit;
  std::unordered_map<Function *, Label *> FnTable;

public:
  CodeGenerator(AssemblyUnit &Unit) : Unit(Unit) {}

  void visit(IRCompilationUnit &IRUnit) override;

  Label *lookupFunctionEntry(Function *Fn);
};

class FunctionCG : public IRVisitor {
  CodeGenerator &CG;
  AssemblyUnit &Unit;
  Procedure &Proc;

  std::unordered_map<Value *, Operand *> ValueTable;
  std::unordered_map<BasicBlock *, Label *> BBTable;

  Label *Prologue = nullptr;
  Label *Epilogue = nullptr;
  // Label *InsertPoint = nullptr;

  int ArgCnt = 0;

public:
  FunctionCG(CodeGenerator &CG, AssemblyUnit &Unit, Procedure &Proc)
      : CG(CG),
        Unit(Unit),
        Proc(Proc) {}

  void visit(Function &Fn) override;
  void visit(Parameter &Arg) override;
  void visit(BasicBlock &BB) override;
  void visit(Constant &C) override;
  void visit(AllocaInst &Inst) override;
  void visit(StoreInst &Inst) override;
  void visit(LoadInst &Inst) override;
  void visit(ArithmeticInst &Inst) override;
  void visit(JumpInst &Inst) override;
  void visit(CJumpInst &Inst) override;
  void visit(CallInst &Inst) override;
  void visit(ReturnInst &Inst) override;
};

} // namespace aarch64

#endif // !TOY_LANG_TARGET_AARCH64_CODE_GENERATOR_H
