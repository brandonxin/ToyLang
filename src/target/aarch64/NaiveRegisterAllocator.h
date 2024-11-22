#ifndef TOY_LANG_TARGET_AARCH64_NAIVE_REGISTER_ALLOCATOR_H
#define TOY_LANG_TARGET_AARCH64_NAIVE_REGISTER_ALLOCATOR_H

#include "target/aarch64/Assembly.h"
#include "target/aarch64/AssemblyUnit.h"
#include <cassert>

namespace aarch64 {

class NaiveRegisterAllocator {
public:
  NaiveRegisterAllocator(AssemblyUnit &Unit, Procedure &Proc)
      : Unit(Unit),
        Proc(Proc) {}

  void run() {
    run(*Proc.getPrologue());
    for (auto &Lbl : Proc.getAllLabels()) {
      if (Lbl.get() == Proc.getPrologue() || Lbl.get() == Proc.getEpilogue())
        continue;
      run(*Lbl);
    }

    run(*Proc.getEpilogue());
  }

  void run(Label &Lbl) {
    std::vector<Operand **> Src, Dst;
    for (auto Iter = Lbl.getAllInsts().begin();
         Iter != Lbl.getAllInsts().end();) {
      Src.clear();
      Dst.clear();
      auto Next = std::next(Iter);
      // Collect all virtual registers used by this instruction
      (*Iter)->collectVirtRegs(Src, Dst);

      // If we are loading to a virtual register, we remember the memory operand
      // and erase this instruction. In this way, we postpone the load until
      // we need that value.
      if ((*Iter)->isLoad() && !Dst.empty()) {
        auto *VReg = static_cast<VirtualRegister *>(*Dst[0]);
        auto *Ptr = static_cast<LDR *>(Iter->get())->getPtr();

        Map[VReg] = static_cast<Memory *>(Ptr);
        Iter = Lbl.getAllInsts().erase(Iter);
        continue;
      }
      for (size_t I = 0; I < Src.size(); ++I) {
        auto *VReg = static_cast<VirtualRegister *>(*Src[I]);
        auto *PReg = Unit.getPhysicsReg(8 + I);

        auto It = Map.find(VReg);
        assert(It != Map.end());

        Proc.emitAt<decltype(Iter), LDR>(Lbl, Iter, PReg, It->second);
        *Src[I] = PReg;
      }

      for (size_t I = 0; I < Dst.size(); ++I) {
        auto *VReg = static_cast<VirtualRegister *>(*Dst[I]);
        auto *PReg = Unit.getPhysicsReg(8 + Src.size() + I);

        auto [It, Res] = Map.emplace(VReg, nullptr);
        if (Res)
          It->second = Proc.allocateStackSlot();

        Proc.emitAt<decltype(Iter), LDR>(Lbl, std::next(Iter), PReg,
                                         It->second);
        *Dst[I] = PReg;
      }
      Iter = Next;
    }
  }

private:
  AssemblyUnit &Unit;
  Procedure &Proc;
  std::unordered_map<VirtualRegister *, Memory *> Map;
};

} // namespace aarch64

#endif // !TOY_LANG_TARGET_AARCH64_NAIVE_REGISTER_ALLOCATOR_H
