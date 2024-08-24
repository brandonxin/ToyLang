#ifndef TOY_LANG_IR_FUNCTION_H
#define TOY_LANG_IR_FUNCTION_H

#include <memory>
#include <vector>

#include "ir/BasicBlock.h"
#include "ir/Constant.h"
#include "ir/Instruction.h"
#include "ir/Value.h"

class Function {
public:
  /// We support only 1 type yet, so there is no need to passing a vector.
  Function(size_t ParamsNum);

  std::vector<std::unique_ptr<Value>> &getParams() { return Parameters; }

  BasicBlock *getEntryBlock() const { return EntryBlock; }

  void setInsertPoint(BasicBlock *B);
  BasicBlock *getCurrInsertPoint() const { return InsertPoint; }

  BasicBlock *makeNewBlock();
  Value *makeConstant(int64_t Val);

  template <typename T, typename... ArgTs>
  Instruction *emit(ArgTs &&...Args) {
    auto Inst = std::make_unique<T>(std::forward<ArgTs>(Args)...);
    auto *Ret = Inst.get();
    InsertPoint->append(std::move(Inst));
    return Ret;
  }

private:
  std::vector<std::unique_ptr<Value>> Parameters;
  std::vector<std::unique_ptr<BasicBlock>> AllBlocks;
  std::vector<std::unique_ptr<Constant>> AllConstants;
  BasicBlock *EntryBlock = nullptr;
  BasicBlock *InsertPoint = nullptr;
  size_t InstCount = 0;
};

#endif // !TOY_LANG_IR_FUNCTION_H
