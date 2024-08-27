#ifndef TOY_LANG_IR_FUNCTION_H
#define TOY_LANG_IR_FUNCTION_H

#include <cassert>
#include <memory>
#include <vector>

#include "ir/Argument.h"
#include "ir/BasicBlock.h"
#include "ir/Constant.h"
#include "ir/IRVisitor.h"
#include "ir/Instruction.h"
#include "ir/Value.h"

class Function {
public:
  /// We support only 1 type yet, so there is no need to passing a vector.
  Function(std::string Name, const std::vector<std::string> &Params);

  void accept(IRVisitor &V) { V.visit(*this); }

  std::string &getName() { return Name; }
  std::vector<std::unique_ptr<Argument>> &getArgs() { return Arguments; }

  BasicBlock *makeEntryBlock() {
    assert(AllBlocks.empty() && "EntryBlock exists");
    makeNewBlock();
    return AllBlocks.front().get();
  }

  BasicBlock *getEntryBlock() const {
    if (AllBlocks.empty())
      return nullptr;

    return AllBlocks.front().get();
  }

  std::vector<std::unique_ptr<BasicBlock>> &getBlocks() { return AllBlocks; }

  void setInsertPoint(BasicBlock *B);
  BasicBlock *getCurrInsertPoint() const { return InsertPoint; }

  BasicBlock *makeNewBlock();
  Constant *makeConstant(int64_t Val);

  template <typename T, typename... ArgTs>
  Instruction *emit(ArgTs &&...Args) {
    auto Inst = makeValue<T>(std::forward<ArgTs>(Args)...);
    auto *Ret = Inst.get();
    InsertPoint->append(std::move(Inst));

    if (Ret->hasResult() && Ret->getName().empty())
      Ret->assignNameByNumber(NextValueID++);

    return Ret;
  }

private:
  template <typename T, typename... Ts>
  std::unique_ptr<T> makeValue(Ts &&...Args) {
    return std::make_unique<T>(std::forward<Ts>(Args)...);
  }

private:
  std::string Name;
  std::vector<std::unique_ptr<Argument>> Arguments;
  std::vector<std::unique_ptr<BasicBlock>> AllBlocks;
  std::vector<std::unique_ptr<Constant>> AllConstants;
  BasicBlock *InsertPoint = nullptr;
  size_t NextValueID = 0;
  size_t NextBBID = 0;
};

#endif // !TOY_LANG_IR_FUNCTION_H
