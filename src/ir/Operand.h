#ifndef TOY_LANG_IR_OPERAND_H
#define TOY_LANG_IR_OPERAND_H

class Operand {
public:
  Operand(const Operand &) = delete;
  Operand(Operand &&) = default;
  virtual ~Operand() = default;

  Operand &operator=(const Operand &) = delete;
  Operand &operator=(Operand &&) = default;
};

#endif // !TOY_LANG_IR_OPERAND_H
