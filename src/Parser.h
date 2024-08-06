#pragma once

#include <cstdio>
#include <map>
#include <memory>
#include <string>

#include "AST.h"
#include "Error.h"

//===----------------------------------------------------------------------===//
// Lexer
//===----------------------------------------------------------------------===//

// The lexer returns tokens [0-255] if it is an unknown character, otherwise one
// of these for known things.
enum Token {
  tok_eof = -1,

  // commands
  tok_def = -2,
  tok_extern = -3,

  // primary
  tok_identifier = -4,
  tok_number = -5,

  // control
  tok_if = -6,
  tok_then = -7,
  tok_else = -8,
  tok_for = -9,
  tok_in = -10,

  // operators
  tok_binary = -11,
  tok_unary = -12,

  // var definition
  tok_var = -13
};

class Lexer {
public:
  Lexer(FILE *Stream) : Stream(Stream) {}

  std::pair<int, int> getLastTokPos() const { return LastTokPos; }

  std::string getIdentifierStr() const { return IdentifierStr; }

  int64_t getNumVal() const { return NumVal; }

  /// gettok - Return the next token from standard input.
  int gettok() {
    // Skip any whitespace.
    while (isspace(LastChar))
      LastChar = getchar();

    LastTokPos = getpos();
    if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
      IdentifierStr = (char)LastChar;

      while (isalnum((LastChar = getchar())))
        IdentifierStr += (char)LastChar;

      if (IdentifierStr == "def")
        return tok_def;
      if (IdentifierStr == "extern")
        return tok_extern;
      if (IdentifierStr == "if")
        return tok_if;
      if (IdentifierStr == "then")
        return tok_then;
      if (IdentifierStr == "else")
        return tok_else;
      if (IdentifierStr == "for")
        return tok_for;
      if (IdentifierStr == "in")
        return tok_in;
      if (IdentifierStr == "binary")
        return tok_binary;
      if (IdentifierStr == "unary")
        return tok_unary;
      if (IdentifierStr == "var")
        return tok_var;
      return tok_identifier;
    }

    if (isdigit(LastChar)) { // Number: [0-9]+
      std::string NumStr;
      do {
        NumStr += (char)LastChar;
        LastChar = getchar();
      } while (isdigit(LastChar));

      NumVal = strtol(NumStr.c_str(), nullptr, 0);
      return tok_number;
    }

    if (LastChar == '#') {
      // Comment until end of line.
      do
        LastChar = getchar();
      while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

      if (LastChar != EOF)
        return gettok();
    }

    // Check for end of file.  Don't eat the EOF.
    if (LastChar == EOF)
      return tok_eof;

    // Otherwise, just return the character as its ascii value.
    int ThisChar = LastChar;
    LastChar = getchar();
    return ThisChar;
  }

private:
  int getchar() {
    auto Char = ::fgetc(Stream);

    if (Char == '\n') {
      row += 1;
      col = 0;
    } else
      col += 1;

    return Char;
  }

  std::pair<int, int> getpos() const { return {row, col}; }

  FILE *Stream;

  int row = 1;
  int col = 0;

  int LastChar = ' ';

  std::pair<int, int> LastTokPos;
  std::string IdentifierStr; // Filled in if tok_identifier
  int64_t NumVal;            // Filled in if tok_number
};

//===----------------------------------------------------------------------===//
// Parser
//===----------------------------------------------------------------------===//

class Parser {
public:
  Parser(FILE *Stream) : L(Stream) {
    // Install standard binary operators.
    // 1 is lowest precedence.
    BinopPrecedence['='] = 2;
    BinopPrecedence['<'] = 10;
    BinopPrecedence['+'] = 20;
    BinopPrecedence['-'] = 20;
    BinopPrecedence['*'] = 40; // highest.
  }

  /// top ::= definition | external | ';'
  void Parse(CompilationUnit &Unit) {
    getNextToken();
    while (true) {
      switch (CurTok) {
      case tok_eof: return;
      case ';': // ignore top-level semicolons.
        getNextToken();
        break;
      case tok_def: HandleDefinition(Unit); break;
      case tok_extern: HandleExtern(Unit); break;
      default: assert(false && "unexpected token"); break;
      }
    }
  }

private:
  Lexer L;

  /// CurTok/getNextToken - Provide a simple token buffer.  CurTok is the
  /// current token the parser is looking at.  getNextToken reads another token
  /// from the lexer and updates CurTok with its results.
  int CurTok;

  int getNextToken() {
    CurTok = L.gettok();

    return CurTok;
  }

  /// BinopPrecedence - This holds the precedence for each binary operator that
  /// is defined.
  std::map<char, int> BinopPrecedence;

  /// GetTokPrecedence - Get the precedence of the pending binary operator
  /// token.
  int GetTokPrecedence() {
    if (!isascii(CurTok))
      return -1;

    // Make sure it's a declared binop.
    int TokPrec = BinopPrecedence[CurTok];
    if (TokPrec <= 0)
      return -1;
    return TokPrec;
  }

  static std::string formatToken(int Tok) {
    if (Tok > 0)
      return fmt::format("'{}'", char(Tok));
    switch (Tok) {
    case tok_eof: return fmt::format("<tok_eof>");
    case tok_def: return fmt::format("<tok_def>");
    case tok_extern: return fmt::format("<tok_extern>");
    case tok_identifier: return fmt::format("<tok_identifier>");
    case tok_number: return fmt::format("<tok_number>");
    case tok_if: return fmt::format("<tok_if>");
    case tok_then: return fmt::format("<tok_then>");
    case tok_else: return fmt::format("<tok_else>");
    case tok_for: return fmt::format("<tok_for>");
    case tok_in: return fmt::format("<tok_in>");
    case tok_binary: return fmt::format("<tok_binary>");
    case tok_unary: return fmt::format("<tok_unary>");
    case tok_var: return fmt::format("<tok_var>");
    default: return fmt::format("<unknown>");
    }
  }

public:
  struct Expected {
    std::vector<std::string> Strs;

    Expected(const std::vector<int> &Tokens) {
      for (auto Tok : Tokens) {
        Strs.push_back(formatToken(Tok));
      }
    }

    Expected(std::vector<std::string> Strs) : Strs(std::move(Strs)) {}
  };

  // struct Actual {
  //   int Token;

  //   Actual(int Token) : Token(Token) {}
  // };

  struct In {
    std::string_view Rule;

    In(std::string_view Rule) : Rule(Rule) {}
  };

  struct After {
    std::string_view Symbol;

    After(std::string_view Symbol) : Symbol(Symbol) {}
  };

private:
  std::nullptr_t logError(std::string_view Str);

  std::nullptr_t logError(const Expected &E, const In &I);

  std::nullptr_t logError(const Expected &E, const After &Af);

  /// numberexpr ::= number
  std::unique_ptr<ExprAST> ParseNumberExpr() {
    auto Result = std::make_unique<NumberExprAST>(L.getNumVal());
    getNextToken(); // consume the number
    return std::move(Result);
  }

  /// parenexpr ::= '(' expression ')'
  std::unique_ptr<ExprAST> ParseParenExpr() {
    getNextToken(); // eat (.
    auto V = ParseExpression();
    if (!V)
      return nullptr;

    if (CurTok != ')') {
      return logError(Expected({')'}), In("parenexpr"));
    }
    getNextToken(); // eat ).
    return V;
  }

  /// identifierexpr
  ///   ::= identifier
  ///   ::= identifier '(' expression* ')'
  std::unique_ptr<ExprAST> ParseIdentifierExpr() {
    std::string IdName = L.getIdentifierStr();

    getNextToken(); // eat identifier.

    if (CurTok != '(') // Simple variable ref.
      return std::make_unique<VariableExprAST>(IdName);

    // Call.
    getNextToken(); // eat (
    std::vector<std::unique_ptr<ExprAST>> Args;
    if (CurTok != ')') {
      while (true) {
        if (auto Arg = ParseExpression())
          Args.push_back(std::move(Arg));
        else
          return nullptr;

        if (CurTok == ')')
          break;

        if (CurTok != ',') {
          return logError(Expected({')', ','}), In("argument list"));
        }
        getNextToken();
      }
    }

    // Eat the ')'.
    getNextToken();

    return std::make_unique<CallExprAST>(IdName, std::move(Args));
  }

  /// ifexpr ::= 'if' expression 'then' expression 'else' expression
  std::unique_ptr<ExprAST> ParseIfExpr() {
    getNextToken(); // eat the if.

    // condition.
    auto Cond = ParseExpression();
    if (!Cond)
      return nullptr;

    if (CurTok != tok_then) {
      return logError(Expected({tok_then}), In("if expression"));
    }
    getNextToken(); // eat the then

    auto Then = ParseExpression();
    if (!Then)
      return nullptr;

    if (CurTok != tok_else) {
      return logError(Expected({tok_else}), In("if expression"));
    }

    getNextToken();

    auto Else = ParseExpression();
    if (!Else)
      return nullptr;

    return std::make_unique<IfExprAST>(std::move(Cond), std::move(Then),
                                       std::move(Else));
  }

  /// forexpr ::= 'for' identifier '=' expr ',' expr (',' expr)? 'in' expression
  std::unique_ptr<ExprAST> ParseForExpr() {
    getNextToken(); // eat the for.

    if (CurTok != tok_identifier) {
      // return LogError("expected identifier after for");
      return logError(Expected({tok_identifier}), After("for"));
    }

    std::string IdName = L.getIdentifierStr();
    getNextToken(); // eat identifier.

    if (CurTok != '=')
      return logError(Expected({'='}), After("for"));
    getNextToken(); // eat '='.

    auto Start = ParseExpression();
    if (!Start)
      return nullptr;
    if (CurTok != ',')
      return logError(Expected({','}), After("for start value"));
    getNextToken();

    auto End = ParseExpression();
    if (!End)
      return nullptr;

    // The step value is optional.
    std::unique_ptr<ExprAST> Step;
    if (CurTok == ',') {
      getNextToken();
      Step = ParseExpression();
      if (!Step)
        return nullptr;
    }

    if (CurTok != tok_in)
      return logError(Expected({tok_in}), After("for"));
    getNextToken(); // eat 'in'.

    auto Body = ParseExpression();
    if (!Body)
      return nullptr;

    return std::make_unique<ForExprAST>(IdName, std::move(Start),
                                        std::move(End), std::move(Step),
                                        std::move(Body));
  }

  /// varexpr ::= 'var' identifier ('=' expression)?
  //                    (',' identifier ('=' expression)?)* 'in' expression
  std::unique_ptr<ExprAST> ParseVarExpr() {
    getNextToken(); // eat the var.

    std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> VarNames;

    // At least one variable name is required.
    if (CurTok != tok_identifier)
      return logError(Expected({tok_identifier}), After("var"));

    while (true) {
      std::string Name = L.getIdentifierStr();
      getNextToken(); // eat identifier.

      // Read the optional initializer.
      std::unique_ptr<ExprAST> Init = nullptr;
      if (CurTok == '=') {
        getNextToken(); // eat the '='.

        Init = ParseExpression();
        if (!Init)
          return nullptr;
      }

      VarNames.push_back(std::make_pair(Name, std::move(Init)));

      // End of var list, exit loop.
      if (CurTok != ',')
        break;
      getNextToken(); // eat the ','.

      if (CurTok != tok_identifier)
        return logError(Expected({tok_identifier}), After("','"));
    }

    // At this point, we have to have 'in'.
    if (CurTok != tok_in)
      return logError(Expected({tok_in}), After("var"));
    getNextToken(); // eat 'in'.

    auto Body = ParseExpression();
    if (!Body)
      return nullptr;

    return std::make_unique<VarExprAST>(std::move(VarNames), std::move(Body));
  }

  /// primary
  ///   ::= identifierexpr
  ///   ::= numberexpr
  ///   ::= parenexpr
  ///   ::= ifexpr
  ///   ::= forexpr
  ///   ::= varexpr
  std::unique_ptr<ExprAST> ParsePrimary() {
    switch (CurTok) {
    default: return logError("unknown token when expecting an expression");
    case tok_identifier: return ParseIdentifierExpr();
    case tok_number: return ParseNumberExpr();
    case '(': return ParseParenExpr();
    case tok_if: return ParseIfExpr();
    case tok_for: return ParseForExpr();
    case tok_var: return ParseVarExpr();
    }
  }

  /// unary
  ///   ::= primary
  ///   ::= '!' unary
  std::unique_ptr<ExprAST> ParseUnary() {
    // If the current token is not an operator, it must be a primary expr.
    if (!isascii(CurTok) || CurTok == '(' || CurTok == ',')
      return ParsePrimary();

    // If this is a unary operator, read it.
    int Opc = CurTok;
    getNextToken();
    if (auto Operand = ParseUnary())
      return std::make_unique<UnaryExprAST>(Opc, std::move(Operand));
    return nullptr;
  }

  /// binoprhs
  ///   ::= ('+' unary)*
  std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
                                         std::unique_ptr<ExprAST> LHS) {
    // If this is a binop, find its precedence.
    while (true) {
      int TokPrec = GetTokPrecedence();

      // If this is a binop that binds at least as tightly as the current binop,
      // consume it, otherwise we are done.
      if (TokPrec < ExprPrec)
        return LHS;

      // Okay, we know this is a binop.
      int BinOp = CurTok;
      getNextToken(); // eat binop

      // Parse the unary expression after the binary operator.
      auto RHS = ParseUnary();
      if (!RHS)
        return nullptr;

      // If BinOp binds less tightly with RHS than the operator after RHS, let
      // the pending operator take RHS as its LHS.
      int NextPrec = GetTokPrecedence();
      if (TokPrec < NextPrec) {
        RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
        if (!RHS)
          return nullptr;
      }

      // Merge LHS/RHS.
      LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS),
                                            std::move(RHS));
    }
  }

  /// expression
  ///   ::= unary binoprhs
  ///
  std::unique_ptr<ExprAST> ParseExpression() {
    auto LHS = ParseUnary();
    if (!LHS)
      return nullptr;

    return ParseBinOpRHS(0, std::move(LHS));
  }

  /// prototype
  ///   ::= id '(' id* ')'
  ///   ::= binary LETTER number? (id, id)
  ///   ::= unary LETTER (id)
  std::unique_ptr<PrototypeAST> ParsePrototype() {
    std::string FnName;

    unsigned Kind = 0; // 0 = identifier, 1 = unary, 2 = binary.
    unsigned BinaryPrecedence = 30;

    switch (CurTok) {
    default: return logError(Expected({tok_identifier}), In("prototype"));
    case tok_identifier:
      FnName = L.getIdentifierStr();
      Kind = 0;
      getNextToken();
      break;
    case tok_unary:
      getNextToken();
      if (!isascii(CurTok))
        return logError(Expected({"unary operator"}),
                        In("programmable unary operators"));
      FnName = "unary";
      FnName += (char)CurTok;
      Kind = 1;
      getNextToken();
      break;
    case tok_binary:
      getNextToken();
      if (!isascii(CurTok))
        return logError(Expected({"binary operator"}),
                        In("programmable binary operators"));
      FnName = "binary";
      FnName += (char)CurTok;
      Kind = 2;
      getNextToken();

      // Read the precedence if present.
      if (CurTok == tok_number) {
        if (L.getNumVal() < 1 || L.getNumVal() > 100)
          return logError("Invalid precedence: must be 1..100");

        BinaryPrecedence = (unsigned)L.getNumVal();
        getNextToken();
      }
      break;
    }

    if (CurTok != '(') {
      return logError(Expected({'('}), In("prototype"));
    }

    std::vector<std::string> ArgNames;
    while (getNextToken() == tok_identifier)
      ArgNames.push_back(L.getIdentifierStr());
    if (CurTok != ')') {
      return logError(Expected({')'}), In("prototype"));
    }

    // success.
    getNextToken(); // eat ')'.

    // Verify right number of names for operator.
    if (Kind && ArgNames.size() != Kind)
      return logError("Invalid number of operands for operator");

    return std::make_unique<PrototypeAST>(FnName, ArgNames, Kind != 0,
                                          BinaryPrecedence);
  }

  /// definition ::= 'def' prototype expression
  std::unique_ptr<FunctionAST> ParseDefinition() {
    getNextToken(); // eat def.
    auto Proto = ParsePrototype();
    if (!Proto)
      return nullptr;

    if (auto E = ParseExpression())
      return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    return nullptr;
  }

  /// external ::= 'extern' prototype
  std::unique_ptr<PrototypeAST> ParseExtern() {
    getNextToken(); // eat extern.
    return ParsePrototype();
  }

  void HandleDefinition(CompilationUnit &U) {
    if (auto FnAST = ParseDefinition()) {
      if (auto &P = FnAST->getProto(); P.isBinaryOp())
        BinopPrecedence[P.getOperatorName()] = P.getBinaryPrecedence();

      U.addFunction(std::move(FnAST));
      // if (auto *FnIR = FnAST->codegen()) {
      //   fprintf(stderr, "Read function definition:");
      //   FnIR->print(errs());
      //   fprintf(stderr, "\n");
      //   ExitOnErr(TheJIT->addModule(
      //       ThreadSafeModule(std::move(TheModule), std::move(TheContext))));
      //   InitializeModuleAndManagers();
      // }
    } else {
      // Skip token for error recovery.
      getNextToken();
    }
  }

  void HandleExtern(CompilationUnit &U) {
    if (auto ProtoAST = ParseExtern()) {
      U.addPrototype(std::move(ProtoAST));
      // if (auto *FnIR = ProtoAST->codegen()) {
      //   fprintf(stderr, "Read extern: ");
      //   FnIR->print(errs());
      //   fprintf(stderr, "\n");
      //   FunctionProtos[ProtoAST->getName()] = std::move(ProtoAST);
      // }
    } else {
      // Skip token for error recovery.
      getNextToken();
    }
  }
};

namespace fmt {
template <>
struct formatter<Parser::Expected> : formatter<std::string_view> {
  auto format(const Parser::Expected &E, format_context &ctx) const
      -> format_context::iterator {
    return fmt::format_to(ctx.out(), "{}",
                          fmt::join(E.Strs.begin(), E.Strs.end(), ", "));
  }
};

} // namespace fmt

inline std::nullptr_t Parser::logError(std::string_view Str) {
  auto TokPos = L.getLastTokPos();
  printError("{}:{}: {}", TokPos.first, TokPos.second, Str);
  return nullptr;
}

inline std::nullptr_t Parser::logError(const Expected &E, const In &I) {
  logError(fmt::format("Expected [{}] in {}, got {}", E, I.Rule,
                       formatToken(CurTok)));
  return nullptr;
}

inline std::nullptr_t Parser::logError(const Expected &E, const After &Af) {
  logError(fmt::format("Expected [{}] after {}, got {}", E, Af.Symbol,
                       formatToken(CurTok)));
  return nullptr;
}
