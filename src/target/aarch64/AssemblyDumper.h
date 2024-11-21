#ifndef TOY_LANG_TARGET_AARCH64_ASSEMBLY_DUMPER_H
#define TOY_LANG_TARGET_AARCH64_ASSEMBLY_DUMPER_H

#include "fmt/format.h"

#include "target/aarch64/Assembly.h"
#include "target/aarch64/AssemblyUnit.h"

namespace aarch64 {

class AssemblyDumper {
  FILE *OS;

public:
  AssemblyDumper(FILE *OS) : OS(OS) {}

  void dump(AssemblyUnit &Unit) {
    for (auto &Proc : Unit.getDefinedProcedures())
      dump(*Proc);
  }

  void dump(Procedure &Proc) {
    // TODO: Output assembly directives

    // Dump all labels
    dump(*Proc.getPrologue());
    for (auto &Lbl : Proc.getAllLabels()) {
      if (Lbl.get() == Proc.getPrologue() || Lbl.get() == Proc.getEpilogue())
        continue;
      dump(*Lbl);
    }

    dump(*Proc.getEpilogue());
  }

  void dump(Label &Lbl) {
    fmt::print(OS, "{}:\n", Lbl.getName());
    for (auto &Inst : Lbl.getAllInsts())
      fmt::print("\t{}\n", Inst->toAsm());
    fmt::print("\n");
  }
};

} // namespace aarch64

#endif // !TOY_LANG_TARGET_AARCH64_ASSEMBLY_DUMPER_H
