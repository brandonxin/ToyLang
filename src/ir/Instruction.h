#ifndef TOY_LANG_IR_INSTRUCTION_H
#define TOY_LANG_IR_INSTRUCTION_H

#include <array>

#include "ir/Value.h"

class Instruction : public Value {
public:
};

class StoreInst : public Instruction {
public:
  StoreInst(Value *Ptr, Value *Val) : Ptr(Ptr), Val(Val) {}

private:
  Value *Ptr;
  Value *Val;
};

class LoadInst : public Instruction {
public:
  LoadInst(Value *Ptr) : Ptr(Ptr) {}

private:
  Value *Ptr;
};

class ArithmeticInst : public Instruction {
public:
  enum class Opcode {
    Add,
    Sub,
    Mul,
  };

  ArithmeticInst(Opcode Opc, Value *LHS, Value *RHS)
      : Opc(Opc),
        Operands{LHS, RHS} {}

private:
  Opcode Opc;
  std::array<Value *, 2> Operands;
};

class ReturnInst : public Instruction {
public:
  ReturnInst(Value *Ret) : Ret(Ret) {}

  bool isTerminator() override { return true; }

private:
  Value *Ret;
};

#endif // !TOY_LANG_IR_INSTRUCTION_H
