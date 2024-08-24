#ifndef TOY_LANG_IR_CONSTANT_H
#define TOY_LANG_IR_CONSTANT_H

#include <cstdint>

#include "ir/Value.h"

class Constant : public Value {
public:
  Constant(int64_t ID, int64_t Val) : Value(ID), Val(Val) {}

private:
  int64_t Val;
};

#endif // !TOY_LANG_IR_CONSTANT_H
