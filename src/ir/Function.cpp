#include "ir/Function.h"

#include <algorithm>
#include <cassert>

#include "ir/AllocaInst.h"
#include "ir/Argument.h"
#include "ir/BasicBlock.h"
#include "ir/BranchInst.h"
#include "ir/CallInst.h"
#include "ir/Constant.h"
#include "ir/Instruction.h"

Function::Function(size_t ParamsNum) {
  Parameters.reserve(ParamsNum);
  while (ParamsNum--)
    Parameters.push_back(std::make_unique<Argument>(NextValueID++));

  AllBlocks.push_back(std::make_unique<BasicBlock>(NextValueID++));
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
  AllBlocks.push_back(std::make_unique<BasicBlock>(NextValueID++));
  return AllBlocks.back().get();
}

Constant *Function::makeConstant(int64_t Val) {
  AllConstants.push_back(std::make_unique<Constant>(NextValueID++, Val));
  return AllConstants.back().get();
}
