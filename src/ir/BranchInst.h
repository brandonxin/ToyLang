#ifndef TOY_LANG_IR_BRANCH_INST_H
#define TOY_LANG_IR_BRANCH_INST_H

#include "ir/BasicBlock.h"
#include "ir/Instruction.h"

class BranchInst : public Instruction {
public:
  BranchInst(std::string Name = "") : Instruction(std::move(Name)) {}

  bool isTerminator() override { return true; }
};

class JumpInst : public BranchInst {
public:
  JumpInst(BasicBlock *Dest, std::string Name = "")
      : BranchInst(std::move(Name)),
        Dest(Dest) {}

  void accept(IRVisitor &V) override { V.visit(*this); }

  BasicBlock *getDest() { return Dest; }

private:
  BasicBlock *Dest;
};

class CJumpInst : public BranchInst {
public:
  CJumpInst(Value *Cond, BasicBlock *IfTrue, BasicBlock *IfElse,
            std::string Name = "")
      : BranchInst(std::move(Name)),
        Cond(Cond),
        IfTrue(IfTrue),
        IfElse(IfElse) {}

  void accept(IRVisitor &V) override { V.visit(*this); }

  Value *getCond() { return Cond; }
  BasicBlock *getTrueBB() { return IfTrue; }
  BasicBlock *getFalseBB() { return IfElse; }

private:
  Value *Cond;
  BasicBlock *IfTrue;
  BasicBlock *IfElse;
};

#endif // !TOY_LANG_IR_BRANCH_INST_H
