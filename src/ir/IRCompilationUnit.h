#ifndef TOY_LANG_IR_COMILATION_UNIT_H
#define TOY_LANG_IR_COMILATION_UNIT_H

#include <map>
#include <memory>

#include "ir/Function.h"
#include "ir/IRVisitor.h"

class IRCompilationUnit {
public:
  void accept(IRVisitor &V) { V.visit(*this); }

  auto begin() const { return AllFunctions.begin(); }
  auto end() const { return AllFunctions.end(); }

  Function *lookupFunction(const std::string &Name);
  Function *makeNewFunction(std::string Name, size_t ParamsNum);

private:
  std::vector<std::unique_ptr<Function>> AllFunctions;
  std::map<std::string, Function *> FunctionTable;
};

#endif // !TOY_LANG_IR_COMILATION_UNIT_H
