#include "IRCompilationUnit.h"
#include "ir/Function.h"

Function *IRCompilationUnit::lookupFunction(const std::string &Name) {
  auto Iter = AllFunctions.find(Name);
  return (Iter == AllFunctions.end()) ? nullptr : Iter->second.get();
}

Function *IRCompilationUnit::makeNewFunction(std::string Name,
                                             size_t ParamsNum) {
  auto [Iter, success] = AllFunctions.emplace(Name, nullptr);
  if (!success)
    return nullptr;

  Iter->second = std::make_unique<Function>(std::move(ParamsNum));
  return Iter->second.get();
}
