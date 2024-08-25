#include "IRCompilationUnit.h"
#include "ir/Function.h"

Function *IRCompilationUnit::lookupFunction(const std::string &Name) {
  auto Iter = FunctionTable.find(Name);
  return (Iter == FunctionTable.end()) ? nullptr : Iter->second;
}

Function *IRCompilationUnit::makeNewFunction(std::string Name,
                                             size_t ParamsNum) {
  auto [Iter, success] = FunctionTable.emplace(Name, nullptr);
  if (!success)
    return nullptr;

  auto NewFn =
      std::make_unique<Function>(std::move(Name), std::move(ParamsNum));

  Iter->second = NewFn.get();
  AllFunctions.emplace_back(std::move(NewFn));
  return Iter->second;
}
