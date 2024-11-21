#include "ir/Function.h"

#include <algorithm>
#include <cassert>

#include "fmt/format.h"

#include "ir/AllocaInst.h"
#include "ir/Argument.h"
#include "ir/BasicBlock.h"
#include "ir/BranchInst.h"
#include "ir/CallInst.h"
#include "ir/Constant.h"
#include "ir/Instruction.h"

Function::Function(std::string Name, const std::vector<std::string> &Params)
    : Name(std::move(Name)) {
  Arguments.reserve(Params.size());
  for (const auto &Param : Params)
    Arguments.push_back(makeValue<Parameter>(Param));

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
  auto *Ret = AllBlocks.back().get();
  if (Ret->getName().empty())
    Ret->assignName(fmt::format("BB_{}", NextBBID++));
  return Ret;
}

Constant *Function::makeConstant(int64_t Val) {
  AllConstants.push_back(makeValue<Constant>(Val));
  return AllConstants.back().get();
}
