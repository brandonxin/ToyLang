#include "ir/BasicBlock.h"

#include <cassert>

void BasicBlock::append(std::unique_ptr<Instruction> Inst) {
  AllInsts.emplace_back(std::move(Inst));
}

Instruction *BasicBlock::getLastInst() {
  assert(!AllInsts.empty() && "Empty BasicBlock");
  return AllInsts.back().get();
}
