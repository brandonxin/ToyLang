#ifndef TOY_LANG_IR_VALUE_H
#define TOY_LANG_IR_VALUE_H

class Value {
public:
  Value(const Value &) = delete;
  Value(Value &&) = default;
  virtual ~Value() = 0;

  Value &operator=(const Value &) = delete;
  Value &operator=(Value &&) = default;

  virtual bool isLValue() = 0;
};

#endif // !TOY_LANG_IR_VALUE_H
