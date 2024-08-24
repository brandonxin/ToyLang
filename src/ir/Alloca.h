#ifndef TOY_LANG_IR_ALLOCA_H
#define TOY_LANG_IR_ALLOCA_H

#include "ir/Instruction.h"

class AllocaInst : public Instruction {
public:
  bool isLValue() override { return true; }
};

#endif // !TOY_LANG_IR_ALLOCA_H