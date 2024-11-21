#ifndef TOY_LANG_IR_TARGET_AARCH64_ASSEMBLY_H
#define TOY_LANG_IR_TARGET_AARCH64_ASSEMBLY_H

#include <map>
#include <memory>
#include <vector>

#include "fmt/format.h"

namespace aarch64 {

class AssemblyUnit;

class Operand {
public:
  virtual ~Operand() = 0;

  virtual bool isMemory() { return false; }
  virtual bool isRegister() { return false; }
  virtual bool isConstant() { return false; }
  virtual bool isLabel() { return false; }

  virtual std::string toAsm() = 0;
};

class Instruction {
public:
  virtual ~Instruction() = 0;

  virtual std::string toAsm() = 0;
};

class Label : public Operand {
  std::string Name;
  std::vector<std::unique_ptr<Instruction>> AllInsts;

public:
  Label(std::string Name) : Name(std::move(Name)) {}

  std::string toAsm() override { return Name; }

  std::string_view getName() const { return Name; }

  void append(std::unique_ptr<Instruction> Inst) {
    // TODO: Add successor if Inst is a branch instruction.
    AllInsts.emplace_back(std::move(Inst));
  }

  const auto &getAllInsts() { return AllInsts; }
};

class Register : public Operand {
  std::string Name;

public:
  Register(std::string Name) : Name(std::move(Name)) {}

  bool isRegister() override { return true; }
  std::string toAsm() override { return Name; }
};

class VirtualRegister : public Register {
public:
  VirtualRegister(std::string Name) : Register(std::move(Name)) {}
};

class PhysicalRegister : public Register {
public:
  PhysicalRegister(std::string Name) : Register(std::move(Name)) {}
};

class Memory : public Operand {
  Register *Base;
  int64_t Offset;

public:
  Memory(Register *Base, int64_t Offset) : Base(Base), Offset(Offset) {}

  bool isMemory() override { return true; }

  std::string toAsm() override;
};

class StackSlot : public Memory {
public:
  StackSlot(PhysicalRegister *SP, int64_t Offset) : Memory(SP, Offset) {}
};

class ImmediateValue : public Operand {
  int64_t Val;

public:
  ImmediateValue(int64_t Val) : Val(Val) {}

  bool isConstant() override { return true; }

  std::string toAsm() override;
};

class MOV : public Instruction {
  Operand *Target;
  Operand *Source;

public:
  MOV(Operand *Target, Operand *Source) : Target(Target), Source(Source) {}

  std::string toAsm() override;
};

class LDR : public Instruction {
  Operand *Reg;
  Operand *Ptr;

public:
  LDR(Operand *Reg, Operand *Ptr) : Reg(Reg), Ptr(Ptr) {}

  std::string toAsm() override;
};

class STR : public Instruction {
  Operand *Val;
  Operand *Ptr;

public:
  STR(Operand *Val, Operand *Ptr) : Val(Val), Ptr(Ptr) {}

  std::string toAsm() override;
};

class B : public Instruction {
  Label *Target;

public:
  B(Label *Target) : Target(Target) {}

  std::string toAsm() override;
};

class CBNZ : public Instruction {
  Operand *Value;
  Label *Target;

public:
  CBNZ(Operand *Value, Label *Target) : Value(Value), Target(Target) {}

  std::string toAsm() override;
};

class BL : public Instruction {
  Label *Target;

public:
  BL(Label *Callee) : Target(Callee) {}

  std::string toAsm() override;
};

class RET : public Instruction {
public:
  std::string toAsm() override;
};

class ADD : public Instruction {
  Operand *Result;
  Operand *LHS;
  Operand *RHS;

public:
  ADD(Operand *Result, Operand *LHS, Operand *RHS)
      : Result(Result),
        LHS(LHS),
        RHS(RHS) {}

  std::string toAsm() override;
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

  std::string toAsm() override;
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

  std::string toAsm() override;
};

class Procedure {
  AssemblyUnit &Unit;
  std::string Name;
  Label *InsertPoint = nullptr;
  Label *Prologue = nullptr;
  Label *Epilogue = nullptr;
  std::vector<std::unique_ptr<Label>> AllLabels;
  std::vector<std::unique_ptr<VirtualRegister>> AllVirtRegs;
  std::vector<std::unique_ptr<ImmediateValue>> AllImms;
  std::vector<std::unique_ptr<StackSlot>> AllSlots;

  int NextLabelIndex = 0;
  int NextVirtualRegisterIndex = 0;

public:
  Procedure(AssemblyUnit &Unit, std::string Name)
      : Unit(Unit),
        Name(std::move(Name)) {
    Prologue = makeNewLabel(fmt::format("_{}", this->Name), false);
    Epilogue = makeNewLabel(fmt::format("epilogue", this->Name));
  }

  std::vector<std::unique_ptr<Label>> &getAllLabels() { return AllLabels; }

  Label *makeNewLabel(std::string LblName, bool Prefix = true);

  Label *getPrologue() { return Prologue; }

  Label *getEpilogue() { return Epilogue; }

  Label *getEntryLabel() { return getPrologue(); }

  void setInsertPoint(Label *Lbl) { InsertPoint = Lbl; }

  ImmediateValue *makeImm(int64_t Val) {
    AllImms.emplace_back(std::make_unique<ImmediateValue>(Val));
    return AllImms.back().get();
  }

  VirtualRegister *makeVirtReg();

  StackSlot *allocateStackSlot();

  template <typename T, typename... Ts>
  void emit(Ts &&...Args) {
    InsertPoint->append(std::make_unique<T>(std::forward<Ts>(Args)...));
  }
};

} // namespace aarch64

#endif // !TOY_LANG_IR_TARGET_AARCH64_ASSEMBLY_H
