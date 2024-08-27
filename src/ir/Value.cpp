#include "Value.h"

#include "fmt/format.h"

Value::Value(std::string Name) : Name(std::move(Name)) {}

Value::~Value() {}

void Value::assignNameByNumber(int64_t ID) {
  Name = fmt::format("%{}", ID);
}
