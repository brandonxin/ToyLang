#include "target/aarch64/Assembly.h"

#include "fmt/format.h"

#include "target/aarch64/AssemblyUnit.h"

namespace aarch64 {

Operand::~Operand() = default;

Instruction::~Instruction() = default;

std::string Memory::toAsm() {
  auto Asm = Base->toAsm();
  return fmt::format("[{}, #{}]", Asm, Offset);
}

std::string ImmediateValue::toAsm() {
  return fmt::format("#{}", Val);
}

std::string MOV::toAsm() {
  return fmt::format("mov\t{}, {}", Target->toAsm(), Source->toAsm());
}

std::string LDR::toAsm() {
  return fmt::format("ldr\t{}, {}", Reg->toAsm(), Ptr->toAsm());
}

std::string STR::toAsm() {
  return fmt::format("str\t{}, {}", Val->toAsm(), Ptr->toAsm());
}

std::string B::toAsm() {
  return fmt::format("b\t{}", Target->toAsm());
}

std::string CBNZ::toAsm() {
  return fmt::format("cbnz\t{}, {}", Value->toAsm(), Target->toAsm());
}

std::string BL::toAsm() {
  return fmt::format("bl\t{}", Target->toAsm());
}

std::string RET::toAsm() {
  return fmt::format("ret");
}

std::string ADD::toAsm() {
  return fmt::format("add\t{}, {}, {}", Result->toAsm(), LHS->toAsm(),
                     RHS->toAsm());
}

std::string SUB::toAsm() {
  return fmt::format("sub\t{}, {}, {}", Result->toAsm(), LHS->toAsm(),
                     RHS->toAsm());
}

std::string MUL::toAsm() {
  return fmt::format("mul\t{}, {}, {}", Result->toAsm(), LHS->toAsm(),
                     RHS->toAsm());
}

Label *Procedure::makeNewLabel(std::string LblName, bool Prefix) {
  if (LblName.empty())
    LblName = fmt::format("BB_{}", NextLabelIndex++);

  if (Prefix)
    LblName = fmt::format("{}_{}", this->Name, LblName);
  auto Lbl = std::unique_ptr<Label>(new Label(std::move(LblName)));
  AllLabels.push_back(std::move(Lbl));
  return AllLabels.back().get();
}

VirtualRegister *Procedure::makeVirtReg() {
  AllVirtRegs.emplace_back(std::make_unique<VirtualRegister>(
      fmt::format("_t{}", NextVirtualRegisterIndex++)));
  return AllVirtRegs.back().get();
}

StackSlot *Procedure::allocateStackSlot() {
  AllSlots.emplace_back(std::make_unique<StackSlot>(Unit.getSP(), -1));
  return AllSlots.back().get();
}

} // namespace aarch64
