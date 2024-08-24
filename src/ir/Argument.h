#ifndef TOY_LANG_IR_ARGUMENT_H
#define TOY_LANG_IR_ARGUMENT_H

#include "ir/Value.h"

class Argument : public Value {
public:
  Argument(int64_t ID) : Value(ID) {}

  bool isLValue() override { return true; }
};

#endif // !TOY_LANG_IR_ARGUMENT_H
