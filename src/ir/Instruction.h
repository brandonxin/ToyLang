#ifndef TOY_LANG_IR_INSTRUCTION_H
#define TOY_LANG_IR_INSTRUCTION_H

#include <array>

#include "ir/Value.h"

class Instruction : public Value {
public:
  Instruction(int64_t ID) : Value(ID) {}
};

class StoreInst : public Instruction {
public:
  StoreInst(int64_t ID, Value *Ptr, Value *Val)
      : Instruction(ID),
        Ptr(Ptr),
        Val(Val) {}

private:
  Value *Ptr;
  Value *Val;
};

class LoadInst : public Instruction {
public:
  LoadInst(int64_t ID, Value *Ptr) : Instruction(ID), Ptr(Ptr) {}

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

  ArithmeticInst(int64_t ID, Opcode Opc, Value *LHS, Value *RHS)
      : Instruction(ID),
        Opc(Opc),
        Operands{LHS, RHS} {}

private:
  Opcode Opc;
  std::array<Value *, 2> Operands;
};

class ReturnInst : public Instruction {
public:
  ReturnInst(int64_t ID, Value *Ret) : Instruction(ID), Ret(Ret) {}

  bool isTerminator() override { return true; }

private:
  Value *Ret;
};

#endif // !TOY_LANG_IR_INSTRUCTION_H
