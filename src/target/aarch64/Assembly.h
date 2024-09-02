#ifndef TOY_LANG_IR_TARGET_AARCH64_ASSEMBLY_H
#define TOY_LANG_IR_TARGET_AARCH64_ASSEMBLY_H

#include <map>
#include <memory>
#include <vector>

namespace aarch64 {

class Operand {
public:
  virtual ~Operand() = 0;

  virtual bool isMemory() { return false; }
  virtual bool isRegister() { return false; }
  virtual bool isConstant() { return false; }
  virtual bool isLabel() { return false; }
};

class Instruction {
public:
  virtual ~Instruction() = 0;
};

class Label : public Operand {
  std::vector<std::unique_ptr<Instruction>> AllInsts;

public:
  void append(std::unique_ptr<Instruction> Inst) {
    // TODO: Add successor if Inst is a branch instruction.
    AllInsts.emplace_back(std::move(Inst));
  }
};

class Register : public Operand {
public:
  bool isRegister() override { return true; }
};

class VirtualRegister : public Register {
public:
};

class PhysicalRegister : public Register {
public:
};

class Memory : public Operand {
public:
  bool isMemory() override { return true; }
};

class StackSlot : public Memory {};

class ImmediateValue : public Operand {
  int64_t Val;

public:
  ImmediateValue(int64_t Val) : Val(Val) {}

  bool isConstant() override { return true; }
};

class MOV : public Instruction {
  Operand *Target;
  Operand *Source;

public:
  MOV(Operand *Target, Operand *Source) : Target(Target), Source(Source) {}
};

class LDR : public Instruction {
  Operand *Target;
  Operand *Source;

public:
  LDR(Operand *Target, Operand *Source) : Target(Target), Source(Source) {}
};

class STR : public Instruction {
  Operand *Source;
  Operand *Target;

public:
  STR(Operand *Source, Operand *Target) : Source(Source), Target(Target) {}
};

class B : public Instruction {
  Label *Target;

public:
  B(Label *Target) : Target(Target) {}
};

class CBNZ : public Instruction {
  Operand *Value;
  Label *Target;

public:
  CBNZ(Operand *Value, Label *Target) : Value(Value), Target(Target) {}
};

class BL : public Instruction {
  Label *Target;

public:
  BL(Label *Callee) : Target(Callee) {}
};

class RET : public Instruction {};

class Add : public Instruction {
  Operand *Result;
  Operand *LHS;
  Operand *RHS;

public:
  Add(Operand *Result, Operand *LHS, Operand *RHS)
      : Result(Result),
        LHS(LHS),
        RHS(RHS) {}
};

class SUB : public Instruction {
  Operand *Result;
  Operand *LHS;
  Operand *RHS;

public:
  SUB(Operand *Result, Operand *LHS, Operand *RHS)
      : Result(Result),
        LHS(LHS),
        RHS(RHS) {}
};

class MUL : public Instruction {
  Operand *Result;
  Operand *LHS;
  Operand *RHS;

public:
  MUL(Operand *Result, Operand *LHS, Operand *RHS)
      : Result(Result),
        LHS(LHS),
        RHS(RHS) {}
};

class Procedure {
  Label *InsertPoint = nullptr;
  Label *Prologue = nullptr;
  Label *Epilogue = nullptr;
  std::vector<std::unique_ptr<Label>> AllLabels;
  std::vector<std::unique_ptr<VirtualRegister>> AllVirtRegs;
  std::vector<std::unique_ptr<ImmediateValue>> AllImms;
  std::vector<std::unique_ptr<StackSlot>> AllSlots;

public:
  Label *makeNewLabel() {
    AllLabels.emplace_back(std::make_unique<Label>());
    return AllLabels.back().get();
  }

  Label *makePrologue() {
    Prologue = makeNewLabel();
    return Prologue;
  }

  Label *makeEpilogue() {
    Epilogue = makeNewLabel();
    return Epilogue;
  }

  Label *getPrologue() { return Prologue; }

  Label *getEpilogue() { return Epilogue; }

  Label *getEntryLabel() { return getPrologue(); }

  void setInsertPoint(Label *Lbl) { InsertPoint = Lbl; }

  ImmediateValue *makeImm(int64_t Val) {
    AllImms.emplace_back(std::make_unique<ImmediateValue>(Val));
    return AllImms.back().get();
  }

  VirtualRegister *makeVirtReg() {
    AllVirtRegs.emplace_back(std::make_unique<VirtualRegister>());
    return AllVirtRegs.back().get();
  }

  StackSlot *allocateStackSlot() {
    AllSlots.emplace_back(std::make_unique<StackSlot>());
    return AllSlots.back().get();
  }

  template <typename T, typename... Ts>
  void emit(Ts &&...Args) {
    InsertPoint->append(std::make_unique<T>(std::forward<Ts>(Args)...));
  }
};

} // namespace aarch64

#endif // !TOY_LANG_IR_TARGET_AARCH64_ASSEMBLY_H
