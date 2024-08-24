#include "ir/Function.h"

#include <algorithm>
#include <cassert>

#include "ir/Alloca.h"
#include "ir/Argument.h"
#include "ir/BasicBlock.h"
#include "ir/BranchInst.h"
#include "ir/CallInst.h"
#include "ir/Constant.h"
#include "ir/Instruction.h"

Function::Function(size_t ParamsNum) {
  Parameters.reserve(ParamsNum);
  while (ParamsNum--)
    Parameters.push_back(std::make_unique<Argument>());

  AllBlocks.push_back(std::make_unique<BasicBlock>());
  EntryBlock = AllBlocks.back().get();
  InsertPoint = AllBlocks.back().get();
}

void Function::setInsertPoint(BasicBlock *B) {
  assert(std::find_if(AllBlocks.begin(), AllBlocks.end(),
                      [B](const auto &Ptr) { return B == Ptr.get(); }) !=
             AllBlocks.end() &&
         "Given BasicBlock does not belong to this Function");

  InsertPoint = B;
}

BasicBlock *Function::makeNewBlock() {
  AllBlocks.push_back(std::make_unique<BasicBlock>());
  return AllBlocks.back().get();
}

// void Function::emitCJump(Value *Cond, BasicBlock *T, BasicBlock *F) {
//   InsertPoint->append(std::make_unique<CJumpOp>(Cond, T, F));
// }

// void Function::emitJump(BasicBlock *B) {
//   InsertPoint->append(std::make_unique<JumpOp>(B));
// }

// Value *Function::emitAlloca() {
//   InsertPoint->append(std::make_unique<Alloca>());
//   return InsertPoint->getLastInst();
// }

// void Function::emitReturn() {
//   InsertPoint->append(std::make_unique<ReturnOp>(makeConstant(0)));
// }

// void Function::appendReturn(Value *Val) {
//   InsertPoint->append(std::make_unique<ReturnOp>(Val));
// }

// Value *Function::makeConstant(int64_t Val) {
//   AllConstants.push_back(std::make_unique<Constant>(Val));
//   return AllConstants.back().get();
// }

// Value *Function::emitAdd(Value *LHS, Value *RHS) {
//   InsertPoint->append(
//       std::make_unique<ArithmeticOp>(ArithmeticOp::Opcode::Add, LHS, RHS));
//   return InsertPoint->getLastInst();
// }

// Value *Function::emitSub(Value *LHS, Value *RHS) {
//   InsertPoint->append(
//       std::make_unique<ArithmeticOp>(ArithmeticOp::Opcode::Sub, LHS, RHS));
//   return InsertPoint->getLastInst();
// }
// Value *Function::emitMul(Value *LHS, Value *RHS) {
//   InsertPoint->append(
//       std::make_unique<ArithmeticOp>(ArithmeticOp::Opcode::Mul, LHS, RHS));
//   return InsertPoint->getLastInst();
// }

// Value *Function::emitLoad(Value *Ptr) {
//   InsertPoint->append(std::make_unique<LoadOp>(Ptr));
//   return InsertPoint->getLastInst();
// }

// void Function::emitStore(Value *Ptr, Value *Val) {
//   InsertPoint->append(std::make_unique<StoreOp>(Ptr, Val));
// }

// Value *Function::appendCall(Function *Callee, std::vector<Value *> Args) {
//   InsertPoint->append(std::make_unique<CallInst>(Callee, Args));
//   return InsertPoint->getLastInst();
// }
