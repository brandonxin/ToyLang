#ifndef TOY_LANG_IR_IR_DUMPER_H
#define TOY_LANG_IR_IR_DUMPER_H

#include "fmt/format.h"

#include "ir/AllocaInst.h"
#include "ir/BranchInst.h"
#include "ir/CallInst.h"
#include "ir/IRCompilationUnit.h"
#include "ir/IRVisitor.h"

class BriefFormatter : public IRVisitor {
public:
  std::string operator()(Value &V) {
    Result.clear();

    V.accept(*this);
    if (Result.empty())
      return std::string(V.getName());

    return std::move(Result);
  }

  void visit(Constant &C) { Result = fmt::format("${}", C.getVal()); }

private:
  std::string Result;
};

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
      fmt::print(OS, "{}", Brief(*Fn.getArgs()[I]));
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
    fmt::print(OS, "{}:\n", BB.getName());
    for (auto &Inst : BB) {
      Inst->accept(*this);
    }
  }

  void visit(AllocaInst &Inst) override {
    fmt::print(OS, "    {} = alloca\n", Inst.getName());
  }

  void visit(StoreInst &Inst) override {
    fmt::print(OS, "    store {}, {}\n", Brief(*Inst.getVal()),
               Brief(*Inst.getPtr()));
  }

  void visit(LoadInst &Inst) override {
    fmt::print(OS, "    {} = load {}\n", Inst.getName(), Brief(*Inst.getPtr()));
  }

  void visit(ArithmeticInst &Inst) override {
    std::string_view Opc;
    switch (Inst.getOpc()) {
    case ArithmeticInst::Opcode::Add: Opc = "add"; break;
    case ArithmeticInst::Opcode::Sub: Opc = "sub"; break;
    case ArithmeticInst::Opcode::Mul: Opc = "mul"; break;
    }

    fmt::print(OS, "    {} = {} {}, {}\n", Inst.getName(), Opc,
               Brief(*Inst.getLHS()), Brief(*Inst.getRHS()));
  }

  void visit(JumpInst &Inst) override {
    fmt::print(OS, "    jump {}\n", Brief(*Inst.getDest()));
  }

  void visit(CJumpInst &Inst) override {
    fmt::print(OS, "    cjump {}, {}, {}\n", Brief(*Inst.getCond()),
               Brief(*Inst.getTrueBB()), Brief(*Inst.getFalseBB()));
  }

  void visit(CallInst &Inst) override {
    fmt::print(OS, "    {} = call @{}(", Inst.getName(),
               Inst.getCallee()->getName());
    for (size_t I = 0; I < Inst.getArguments().size(); ++I) {
      if (I != 0)
        fmt::print(OS, ", ");
      fmt::print(OS, "{}", Brief(*Inst.getArguments()[I]));
    }
    fmt::print(OS, ")\n");
  }

  void visit(ReturnInst &Inst) override {
    fmt::print(OS, "    return");
    if (auto *Val = Inst.getVal())
      fmt::print(OS, " {}", Brief(*Val));
    fmt::print(OS, "\n");
  }

private:
  std::FILE *OS;

  BriefFormatter Brief;
};

#endif // !TOY_LANG_IR_IR_DUMPER_H
