#ifndef TOY_LANG_IR_VALUE_H
#define TOY_LANG_IR_VALUE_H

#include <cstdint>
#include <string>

#include "ir/IRVisitor.h"

class Value {
public:
  // Value(int64_t ID);
  Value() = default;
  Value(std::string Name);
  Value(const Value &) = delete;
  Value &operator=(const Value &) = delete;

  Value(Value &&) = default;
  Value &operator=(Value &&) = default;

  virtual ~Value() = 0;

  virtual void accept(IRVisitor &V) = 0;

  virtual bool hasResult() { return false; }
  virtual bool isLValue() { return false; }
  virtual bool isTerminator() { return false; }

  void assignName(std::string Name) { this->Name = Name; }
  void assignNameByNumber(int64_t Num);
  std::string_view getName() const { return Name; }

private:
  std::string Name;
};

#endif // !TOY_LANG_IR_VALUE_H
