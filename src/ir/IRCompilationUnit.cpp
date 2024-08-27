#include "IRCompilationUnit.h"
#include "ir/Function.h"

Function *IRCompilationUnit::lookupFunction(const std::string &Name) {
  auto Iter = FunctionTable.find(Name);
  return (Iter == FunctionTable.end()) ? nullptr : Iter->second;
}

Function *
IRCompilationUnit::makeNewFunction(std::string Name,
                                   const std::vector<std::string> &Params) {
  auto [Iter, success] = FunctionTable.emplace(Name, nullptr);
  if (!success)
    return nullptr;

  auto NewFn = std::make_unique<Function>(std::move(Name), Params);

  Iter->second = NewFn.get();
  AllFunctions.emplace_back(std::move(NewFn));
  return Iter->second;
}
