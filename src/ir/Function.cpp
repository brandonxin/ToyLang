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

Function::Function(std::string Name, size_t ParamsNum) : Name(std::move(Name)) {
  Arguments.reserve(ParamsNum);
  while (ParamsNum--)
    Arguments.push_back(makeValue<Argument>());

  // DO NOT create EntryBlock now, because it can be a external linkage
  // function.
}

void Function::setInsertPoint(BasicBlock *B) {
  assert(std::find_if(AllBlocks.begin(), AllBlocks.end(),
                      [B](const auto &Ptr) { return B == Ptr.get(); }) !=
             AllBlocks.end() &&
         "Given BasicBlock does not belong to this Function");

  InsertPoint = B;
}

BasicBlock *Function::makeNewBlock() {
  AllBlocks.push_back(makeValue<BasicBlock>());
  return AllBlocks.back().get();
}

Constant *Function::makeConstant(int64_t Val) {
  AllConstants.push_back(makeValue<Constant>(Val));
  return AllConstants.back().get();
}
