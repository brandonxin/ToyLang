#ifndef TOY_LANG_IR_FUNCTION_H
#define TOY_LANG_IR_FUNCTION_H

#include <memory>
#include <vector>

#include "ir/BasicBlock.h"
#include "ir/Value.h"

class Function {
public:
  Function(size_t ParamesNum);

  std::vector<std::unique_ptr<Value>> getParams();

  BasicBlock *createEntryBlock();
  BasicBlock *getEntryBlock() const { return EntryBlock; }

  void setInsertPoint(BasicBlock *B);
  BasicBlock *getCurrInsertPoint();

  BasicBlock *makeNewBlock();

  Value *appendCJump(Value *Cond, BasicBlock *T, BasicBlock *F);
  Value *appendJump(BasicBlock *B);
  Value *apendAlloca();
  void appendReturn();
  void appendReturn(Value *Val);
  Value *makeConstant(int64_t Val);

  Value *appendAdd(Value *LHS, Value *RHS);
  Value *appendSub(Value *LHS, Value *RHS);
  Value *appendMul(Value *LHS, Value *RHS);
  Value *appendLoad(Value *Alloca);
  void appendStore(Value *Alloca, Value *Val);
  Value *appendCall(Function *Callee, std::vector<Value *> Args);

private:
  std::vector<std::unique_ptr<Value>> Parameters;
  std::vector<std::unique_ptr<BasicBlock>> AllBlocks;
  BasicBlock *EntryBlock = nullptr;
  BasicBlock *InsertPoint = nullptr;
};

#endif // !TOY_LANG_IR_FUNCTION_H
