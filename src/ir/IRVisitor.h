#ifndef TOY_LANG_IR_IR_VISITOR_H
#define TOY_LANG_IR_IR_VISITOR_H

class IRCompilationUnit;
class Function;
class Argument;
class BasicBlock;
class Constant;

class AllocaInst;
class StoreInst;
class LoadInst;
class ArithmeticInst;
class JumpInst;
class CJumpInst;
class CallInst;
class ReturnInst;

class IRVisitor {
public:
  virtual ~IRVisitor() = default;

  virtual void visit(IRCompilationUnit &IRUnit) {}
  virtual void visit(Function &Fn) {}
  virtual void visit(Argument &Arg) {}
  virtual void visit(BasicBlock &BB) {}
  virtual void visit(Constant &Inst) {}

  virtual void visit(AllocaInst &Inst) {}
  virtual void visit(StoreInst &Inst) {}
  virtual void visit(LoadInst &Inst) {}
  virtual void visit(ArithmeticInst &Inst) {}
  virtual void visit(JumpInst &Inst) {}
  virtual void visit(CJumpInst &Inst) {}
  virtual void visit(CallInst &Inst) {}
  virtual void visit(ReturnInst &Inst) {}
};

#endif // !TOY_LANG_IR_IR_VISITOR_H
