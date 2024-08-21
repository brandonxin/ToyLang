#pragma once

#include "fmt/color.h"
#include "fmt/format.h"

template <typename... T>
void printError(fmt::format_string<T...> fmt, T &&...args) {
  fmt::print(stderr, "toyc: ");
  fmt::print(stderr, fmt::emphasis::bold | fg(fmt::color::red), "error: ");
  fmt::println(fmt, std::forward<T>(args)...);
}
