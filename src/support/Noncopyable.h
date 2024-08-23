#ifndef TOY_LANG_SUPPORT_NONCOPYABLE_H
#define TOY_LANG_SUPPORT_NONCOPYABLE_H

class Noncopyable {
public:
  Noncopyable() = default;
  ~Noncopyable() = default;

  Noncopyable(const Noncopyable &) = delete;
  Noncopyable &operator=(const Noncopyable &) = delete;
};

#endif // !TOY_LANG_SUPPORT_NONCOPYABLE_H
