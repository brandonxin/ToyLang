#ifndef TOY_LANG_IR_VALUE_H
#define TOY_LANG_IR_VALUE_H

class Value {
public:
  Value() = default;
  Value(const Value &) = delete;
  Value(Value &&) = default;
  virtual ~Value() = 0;

  Value &operator=(const Value &) = delete;
  Value &operator=(Value &&) = default;

  virtual bool isLValue() { return false; }
  virtual bool isTerminator() { return false; }
};

#endif // !TOY_LANG_IR_VALUE_H
