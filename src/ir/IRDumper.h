#ifndef TOY_LANG_IR_IR_DUMPER_H
#define TOY_LANG_IR_IR_DUMPER_H

#include "fmt/format.h"

#include "ir/AllocaInst.h"
#include "ir/BranchInst.h"
#include "ir/CallInst.h"
#include "ir/IRCompilationUnit.h"
#include "ir/IRVisitor.h"

//===----------------------------------------------------------------------===//
// TODO:
// - [ ] ID is not neccessary for Value. Value needs a Name. ID is for naming.
// - [ ] Different Value Class should have different number space.
// - [ ] Better IRDumper. Display Name. Display Constant's value.
//===----------------------------------------------------------------------===//

class IRDumper : public IRVisitor {
public:
  IRDumper(std::FILE *OS, IRCompilationUnit &IRUnit) : OS(OS) {
    IRUnit.accept(*this);
  }

  void visit(IRCompilationUnit &IRUnit) override {
    for (auto &Fn : IRUnit) {
      Fn->accept(*this);
    }
  }

  void visit(Function &Fn) override {
    if (Fn.getEntryBlock())
      fmt::print(OS, "define @{}(", Fn.getName());
    else
      fmt::print(OS, "extern @{}(", Fn.getName());

    for (size_t I = 0; I < Fn.getArgs().size(); ++I) {
      if (I != 0)
        fmt::print(OS, ", ");
      fmt::print(OS, "%{}", Fn.getArgs()[I]->getID());
    }
    fmt::print(OS, ")");

    if (!Fn.getEntryBlock()) {
      fmt::print(OS, ";\n");
      return;
    }

    fmt::print(" {{\n");

    for (auto &BB : Fn.getBlocks()) {
      BB->accept(*this);
    }
    fmt::print("}}\n");
  }

  void visit(BasicBlock &BB) override {
    fmt::print(OS, "BB_{}:\n", BB.getID());
    for (auto &Inst : BB) {
      Inst->accept(*this);
    }
  }

  //   void visit(Constant &C) override {}

  void visit(AllocaInst &Inst) override {
    fmt::print(OS, "    %{} = alloca\n", Inst.getID());
  }

  void visit(StoreInst &Inst) override {
    fmt::print(OS, "    %{} : store %{}, %{}\n", Inst.getID(),
               Inst.getVal()->getID(), Inst.getPtr()->getID());
  }

  void visit(LoadInst &Inst) override {
    fmt::print(OS, "    %{} = load %{}\n", Inst.getID(),
               Inst.getPtr()->getID());
  }

  void visit(ArithmeticInst &Inst) override {
    std::string_view Opc;
    switch (Inst.getOpc()) {
    case ArithmeticInst::Opcode::Add: Opc = "add"; break;

    case ArithmeticInst::Opcode::Sub: Opc = "sub"; break;
    case ArithmeticInst::Opcode::Mul: Opc = "mul"; break;
    }

    fmt::print(OS, "    %{} = {} %{}, %{}\n", Inst.getID(), Opc,
               Inst.getLHS()->getID(), Inst.getRHS()->getID());
  }

  void visit(JumpInst &Inst) override {
    fmt::print(OS, "    %{} : jump BB_{}\n", Inst.getID(),
               Inst.getDest()->getID());
  }

  void visit(CJumpInst &Inst) override {
    fmt::print(OS, "    %{} : cjump %{}, BB_{}, BB_{}\n", Inst.getID(),
               Inst.getCond()->getID(), Inst.getTrueBB()->getID(),
               Inst.getFalseBB()->getID());
  }

  void visit(CallInst &Inst) override {
    fmt::print(OS, "    %{} = call @{}(", Inst.getID(),
               Inst.getCallee()->getName());
    for (size_t I = 0; I < Inst.getArguments().size(); ++I) {
      if (I != 0)
        fmt::print(OS, ", ");
      fmt::print(OS, "%{}", Inst.getArguments()[I]->getID());
    }
    fmt::print(OS, ")\n");
  }

  void visit(ReturnInst &Inst) override {
    fmt::print(OS, "    %{} : return", Inst.getID());
    if (auto *Val = Inst.getVal())
      fmt::print(OS, " %{}", Val->getID());
    fmt::print(OS, "\n");
  }

private:
  std::FILE *OS;
};

#endif // !TOY_LANG_IR_IR_DUMPER_H
