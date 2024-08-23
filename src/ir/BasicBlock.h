#ifndef TOY_LANG_IR_BASIC_BLOCK_H
#define TOY_LANG_IR_BASIC_BLOCK_H

#include <vector>

#include "ir/Operation.h"
#include "ir/Value.h"

class BasicBlock : public Value {
public:
private:
  std::vector<BasicBlock *> Preds;
  std::vector<BasicBlock *> Succs;

  std::vector<std::unique_ptr<Operation>> AllOps;
};

#endif // !TOY_LANG_IR_BASIC_BLOCK_H
