#ifndef TOY_LANG_IR_ARGUMENT_H
#define TOY_LANG_IR_ARGUMENT_H

#include "ir/Value.h"

class Parameter : public Value {
public:
  Parameter(std::string Name) : Value(std::move(Name)) {}

  void accept(IRVisitor &V) override { V.visit(*this); }

  bool isLValue() override { return true; }
};

#endif // !TOY_LANG_IR_ARGUMENT_H
