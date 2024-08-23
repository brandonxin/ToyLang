#ifndef TOY_LANG_IR_OPERATION_H
#define TOY_LANG_IR_OPERATION_H

class Operation {
public:
  Operation(const Operation &) = delete;
  Operation(Operation &&) = default;
  virtual ~Operation() = 0;

  Operation &operator=(const Operation &) = delete;
  Operation &operator=(Operation &&) = default;
};

#endif /* TOY_LANG_IR_OPERATION_H */
