#ifndef TOY_LANG_IR_BRANCH_INST_H
#define TOY_LANG_IR_BRANCH_INST_H

#include "ir/BasicBlock.h"
#include "ir/Instruction.h"

class BranchInst : public Instruction {
public:
  BranchInst(int64_t ID) : Instruction(ID) {}

  bool isTerminator() override { return true; }
};

class JumpInst : public BranchInst {
public:
  JumpInst(int64_t ID, BasicBlock *Dest) : BranchInst(ID), Dest(Dest) {}

  void accept(IRVisitor &V) override { V.visit(*this); }

  BasicBlock *getDest() { return Dest; }

private:
  BasicBlock *Dest;
};

class CJumpInst : public BranchInst {
public:
  CJumpInst(int64_t ID, Value *Cond, BasicBlock *IfTrue, BasicBlock *IfElse)
      : BranchInst(ID),
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
