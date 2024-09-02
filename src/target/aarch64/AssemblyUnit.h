#ifndef TOY_LANG_TARGET_AARCH64_ASSEMBLY_UNIT_H
#define TOY_LANG_TARGET_AARCH64_ASSEMBLY_UNIT_H

#include "target/aarch64/Assembly.h"

namespace aarch64 {

class AssemblyUnit {
  std::vector<std::unique_ptr<Procedure>> DefinedProcedures;
  std::vector<std::unique_ptr<Label>> ExternalProcedures;

public:
  Procedure *makeNewProcedure() {
    DefinedProcedures.emplace_back(std::make_unique<Procedure>());
    return DefinedProcedures.back().get();
  }

  Label *addExternalProcedure(std::string Name) {
    ExternalProcedures.emplace_back(std::make_unique<Label>());
    return ExternalProcedures.back().get();
  }
};

} // namespace aarch64

#endif // !TOY_LANG_TARGET_AARCH64_ASSEMBLY_UNIT_H
