#ifndef TOY_LANG_IR_COMILATION_UNIT_H
#define TOY_LANG_IR_COMILATION_UNIT_H

#include <map>
#include <memory>

#include "ir/Function.h"

class IRCompilationUnit {
public:
  Function *lookupFunction(const std::string &Name);
  Function *makeNewFunction(std::string Name,
                            std::vector<std::string> ParamsName);

private:
  std::map<std::string, std::unique_ptr<Function>> AllFunctions;
};

#endif // !TOY_LANG_IR_COMILATION_UNIT_H
