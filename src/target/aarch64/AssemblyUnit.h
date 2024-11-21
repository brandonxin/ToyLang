#ifndef TOY_LANG_TARGET_AARCH64_ASSEMBLY_UNIT_H
#define TOY_LANG_TARGET_AARCH64_ASSEMBLY_UNIT_H

#include "target/aarch64/Assembly.h"

namespace aarch64 {

class AssemblyUnit {
  std::array<PhysicalRegister, 32> AllPhysicsRegs = {
      // https://developer.arm.com/documentation/102374/0102/Registers-in-AArch64---general-purpose-registers
      // 31 general purpose registers. Each register can be used as a 64-bit X
      // register (X0..X30), or as a 32-bit W register (W0..W30).
      PhysicalRegister("x0"),
      PhysicalRegister("x1"),
      PhysicalRegister("x2"),
      PhysicalRegister("x3"),
      PhysicalRegister("x4"),
      PhysicalRegister("x5"),
      PhysicalRegister("x6"),
      PhysicalRegister("x7"),
      PhysicalRegister("x8"),
      PhysicalRegister("x9"),
      PhysicalRegister("x10"),
      PhysicalRegister("x11"),
      PhysicalRegister("x12"),
      PhysicalRegister("x13"),
      PhysicalRegister("x14"),
      PhysicalRegister("x15"),
      PhysicalRegister("x16"),
      PhysicalRegister("x17"),
      PhysicalRegister("x18"),
      PhysicalRegister("x19"),
      PhysicalRegister("x20"),
      PhysicalRegister("x21"),
      PhysicalRegister("x22"),
      PhysicalRegister("x23"),
      PhysicalRegister("x24"),
      PhysicalRegister("x25"),
      PhysicalRegister("x26"),
      PhysicalRegister("x27"),
      PhysicalRegister("x28"),
      // NOTE x29 can also be referred as "fp"
      PhysicalRegister("x29"),
      // NOTE x30 can also be referred as "lr"
      PhysicalRegister("x30"),
      PhysicalRegister("sp"),
  };
  std::vector<std::unique_ptr<Procedure>> DefinedProcedures;
  std::vector<std::unique_ptr<Label>> ExternalProcedures;

public:
  PhysicalRegister *getPhysicsReg(int N) {
    if (N < 0 || N > 30)
      return nullptr;

    return &AllPhysicsRegs[N];
  }

  PhysicalRegister *getSP() { return &AllPhysicsRegs[31]; }

  std::vector<std::unique_ptr<Procedure>> &getDefinedProcedures() {
    return DefinedProcedures;
  }

  Procedure *makeNewProcedure(std::string ProcName) {
    DefinedProcedures.emplace_back(
        std::make_unique<Procedure>(*this, std::move(ProcName)));
    return DefinedProcedures.back().get();
  }

  Label *addExternalProcedure(std::string ProcName) {
    ExternalProcedures.emplace_back(
        std::make_unique<Label>(std::move(ProcName)));
    return ExternalProcedures.back().get();
  }
};

} // namespace aarch64

#endif // !TOY_LANG_TARGET_AARCH64_ASSEMBLY_UNIT_H
