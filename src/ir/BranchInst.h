#ifndef TOY_LANG_IR_BRANCH_INST_H
#define TOY_LANG_IR_BRANCH_INST_H

#include "ir/BasicBlock.h"
#include "ir/Instruction.h"

class BranchInst : public Instruction {
public:
  bool isTerminator() override { return true; }
};

class JumpInst : public BranchInst {
public:
  JumpInst(BasicBlock *Dest) : Dest(Dest) {}

private:
  BasicBlock *Dest;
};

class CJumpInst : public BranchInst {
public:
  CJumpInst(Value *Cond, BasicBlock *IfTrue, BasicBlock *IfElse)
      : Cond(Cond),
        IfTrue(IfTrue),
        IfElse(IfElse) {}

private:
  Value *Cond;
  BasicBlock *IfTrue;
  BasicBlock *IfElse;
};

#endif // !TOY_LANG_IR_BRANCH_INST_H
