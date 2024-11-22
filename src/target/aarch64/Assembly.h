#ifndef TOY_LANG_IR_TARGET_AARCH64_ASSEMBLY_H
#define TOY_LANG_IR_TARGET_AARCH64_ASSEMBLY_H

#include <list>
#include <map>
#include <memory>
#include <vector>

#include "fmt/format.h"

namespace aarch64 {

class AssemblyUnit;
class VirtualRegister;

class Operand {
public:
  virtual ~Operand() = 0;

  virtual bool isMemory() { return false; }
  virtual bool isRegister() { return false; }
  virtual bool isConstant() { return false; }
  virtual bool isLabel() { return false; }
  virtual bool isVirtual() { return false; }
  virtual bool isPhysical() { return false; }
  virtual void collectVirtRegs(std::vector<Operand **> & /*Src*/) {}

  virtual std::string toAsm() = 0;
};

class Instruction {
public:
  virtual ~Instruction() = 0;

  virtual bool isLoad() { return false; }

  virtual std::string toAsm() = 0;
  virtual void collectVirtRegs(std::vector<Operand **> & /*Src*/,
                               std::vector<Operand **> & /*Dst*/) {}
};

class Label : public Operand {
  std::string Name;
  std::list<std::unique_ptr<Instruction>> AllInsts;

public:
  Label(std::string Name) : Name(std::move(Name)) {}

  std::string toAsm() override { return Name; }

  std::string_view getName() const { return Name; }

  void append(std::unique_ptr<Instruction> Inst) {
    // TODO: Add successor if Inst is a branch instruction.
    AllInsts.emplace_back(std::move(Inst));
  }

  const auto &getAllInsts() const { return AllInsts; }
  auto &getAllInsts() { return AllInsts; }
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

  bool isVirtual() override { return true; }
};

class PhysicalRegister : public Register {
public:
  PhysicalRegister(std::string Name) : Register(std::move(Name)) {}

  bool isPhysical() override { return true; }
};

class Memory : public Operand {
  Operand *Base;
  int64_t Offset;

public:
  Memory(Operand *Base, int64_t Offset) : Base(Base), Offset(Offset) {}

  bool isMemory() override { return true; }

  std::string toAsm() override;

  void collectVirtRegs(std::vector<Operand **> &Src) override {
    if (Base->isVirtual())
      Src.push_back(&Base);
  }
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

  void collectVirtRegs(std::vector<Operand **> &Src,
                       std::vector<Operand **> &Dst) override {
    if (Source->isVirtual())
      Src.push_back(&Source);
    else if ((Source)->isMemory())
      Source->collectVirtRegs(Src);

    if (Target->isVirtual())
      Dst.push_back(&Target);
    else if ((Target)->isMemory()) {
      // NOTE If Target includes a regsiter, then this register will be read,
      // not be written, thus it should be collected into Src.
      Target->collectVirtRegs(Src);
    }
  }
};

class LDR : public Instruction {
  Operand *Reg;
  Operand *Ptr;

public:
  LDR(Operand *Reg, Operand *Ptr) : Reg(Reg), Ptr(Ptr) {}

  bool isLoad() override { return true; }

  std::string toAsm() override;

  void collectVirtRegs(std::vector<Operand **> &Src,
                       std::vector<Operand **> &Dst) override {
    Ptr->collectVirtRegs(Src);

    if (Reg->isVirtual())
      Dst.push_back(&Reg);
  }

  Operand *getReg() { return Reg; }
  Operand *getPtr() { return Ptr; }
};

class STR : public Instruction {
  Operand *Val;
  Operand *Ptr;

public:
  STR(Operand *Val, Operand *Ptr) : Val(Val), Ptr(Ptr) {}

  std::string toAsm() override;

  void collectVirtRegs(std::vector<Operand **> &Src,
                       std::vector<Operand **> &Dst) override {
    if (Val->isVirtual())
      Src.push_back(&Val);

    // NOTE If Target includes a regsiter, then this register will be read,
    // not be written, thus it should be collected into Src.
    Ptr->collectVirtRegs(Src);
  }
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

  void collectVirtRegs(std::vector<Operand **> &Src,
                       std::vector<Operand **> & /*Dst*/) override {
    if (Value->isVirtual())
      Src.push_back(&Value);
    else if (Value->isMemory())
      Value->collectVirtRegs(Src);
  }
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
  void collectVirtRegs(std::vector<Operand **> &Src,
                       std::vector<Operand **> &Dst) override {
    for (auto Op : {&LHS, &RHS}) {
      if ((*Op)->isVirtual())
        Src.push_back(Op);
      else if ((*Op)->isMemory())
        (*Op)->collectVirtRegs(Src);
    }

    if (Result->isVirtual())
      Dst.push_back(&Result);
  }
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
  void collectVirtRegs(std::vector<Operand **> &Src,
                       std::vector<Operand **> &Dst) override {
    for (auto Op : {&LHS, &RHS}) {
      if ((*Op)->isVirtual())
        Src.push_back(Op);
      else if ((*Op)->isMemory())
        (*Op)->collectVirtRegs(Src);
    }

    if (Result->isVirtual())
      Dst.push_back(&Result);
  }
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
  void collectVirtRegs(std::vector<Operand **> &Src,
                       std::vector<Operand **> &Dst) override {
    for (auto Op : {&LHS, &RHS}) {
      if ((*Op)->isVirtual())
        Src.push_back(Op);
      else if ((*Op)->isMemory())
        (*Op)->collectVirtRegs(Src);
    }

    if (Result->isVirtual())
      Dst.push_back(&Result);
  }
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

  template <typename IterT, typename T, typename... Ts>
  void emitAt(Label &Lbl, IterT Iter, Ts &&...Args) {
    // InsertPoint->append(std::make_unique<T>(std::forward<Ts>(Args)...));
    Lbl.getAllInsts().insert(Iter,
                             std::make_unique<T>(std::forward<Ts>(Args)...));
  }
};

} // namespace aarch64

#endif // !TOY_LANG_IR_TARGET_AARCH64_ASSEMBLY_H
