#ifndef TOY_LANG_IR_VALUE_H
#define TOY_LANG_IR_VALUE_H

#include <cstdint>

#include "ir/IRVisitor.h"

class Value {
public:
  Value(int64_t ID) : ID(ID) {}
  Value(const Value &) = delete;
  Value &operator=(const Value &) = delete;

  Value(Value &&) = default;
  Value &operator=(Value &&) = default;

  virtual ~Value() = 0;

  virtual void accept(IRVisitor &V) = 0;

  virtual bool isLValue() { return false; }
  virtual bool isTerminator() { return false; }

  int64_t getID() const { return ID; }

private:
  int64_t ID;
};

#endif // !TOY_LANG_IR_VALUE_H
