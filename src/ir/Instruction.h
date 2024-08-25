#ifndef TOY_LANG_IR_INSTRUCTION_H
#define TOY_LANG_IR_INSTRUCTION_H

#include <array>

#include "ir/Value.h"

class Instruction : public Value {
public:
  Instruction(int64_t ID) : Value(ID) {}

  virtual void accept(IRVisitor &V) = 0;
};

class StoreInst : public Instruction {
public:
  StoreInst(int64_t ID, Value *Ptr, Value *Val)
      : Instruction(ID),
        Ptr(Ptr),
        Val(Val) {}

  void accept(IRVisitor &V) override { V.visit(*this); }

  Value *getPtr() { return Ptr; }
  Value *getVal() { return Val; }

private:
  Value *Ptr;
  Value *Val;
};

class LoadInst : public Instruction {
public:
  LoadInst(int64_t ID, Value *Ptr) : Instruction(ID), Ptr(Ptr) {}

  void accept(IRVisitor &V) override { V.visit(*this); }

  Value *getPtr() { return Ptr; }

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

  void accept(IRVisitor &V) override { V.visit(*this); }

  Value *getLHS() { return Operands[0]; }
  Value *getRHS() { return Operands[1]; }
  Opcode getOpc() { return Opc; }

private:
  Opcode Opc;
  std::array<Value *, 2> Operands;
};

class ReturnInst : public Instruction {
public:
  ReturnInst(int64_t ID, Value *Ret) : Instruction(ID), Ret(Ret) {}

  void accept(IRVisitor &V) override { V.visit(*this); }

  bool isTerminator() override { return true; }

  Value *getVal() { return Ret; }

private:
  Value *Ret;
};

#endif // !TOY_LANG_IR_INSTRUCTION_H
