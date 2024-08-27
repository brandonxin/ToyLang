#ifndef TOY_LANG_IR_CONSTANT_H
#define TOY_LANG_IR_CONSTANT_H

#include <cstdint>

#include "ir/Value.h"

class Constant : public Value {
public:
  Constant(int64_t Val) : Val(Val) {}

  void accept(IRVisitor &V) { V.visit(*this); }

  int64_t getVal() const { return Val; }

private:
  int64_t Val;
};

#endif // !TOY_LANG_IR_CONSTANT_H
