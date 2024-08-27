#ifndef TOY_LANG_IR_INSTRUCTION_H
#define TOY_LANG_IR_INSTRUCTION_H

#include <array>

#include "ir/Value.h"

class Instruction : public Value {
public:
  Instruction(std::string Name = "") : Value(std::move(Name)) {}

  virtual void accept(IRVisitor &V) = 0;
};

class StoreInst : public Instruction {
public:
  StoreInst(Value *Ptr, Value *Val, std::string Name = "")
      : Instruction(std::move(Name)),
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
  LoadInst(Value *Ptr, std::string Name = "") : Instruction(Name), Ptr(Ptr) {}

  void accept(IRVisitor &V) override { V.visit(*this); }

  bool hasResult() override { return true; }

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

  ArithmeticInst(Opcode Opc, Value *LHS, Value *RHS, std::string Name = "")
      : Instruction(Name),
        Opc(Opc),
        Operands{LHS, RHS} {}

  void accept(IRVisitor &V) override { V.visit(*this); }

  bool hasResult() override { return true; }

  Value *getLHS() { return Operands[0]; }
  Value *getRHS() { return Operands[1]; }
  Opcode getOpc() { return Opc; }

private:
  Opcode Opc;
  std::array<Value *, 2> Operands;
};

class ReturnInst : public Instruction {
public:
  ReturnInst(Value *Ret, std::string Name = "")
      : Instruction(std::move(Name)),
        Ret(Ret) {}

  void accept(IRVisitor &V) override { V.visit(*this); }

  bool isTerminator() override { return true; }
  bool hasResult() override { return true; }

  Value *getVal() { return Ret; }

private:
  Value *Ret;
};

#endif // !TOY_LANG_IR_INSTRUCTION_H
