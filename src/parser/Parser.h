#pragma once

#include <cstdio>
#include <map>
#include <memory>
#include <string>

#include "parser/AST.h"
#include "parser/Error.h"

//===----------------------------------------------------------------------===//
// Lexer
//===----------------------------------------------------------------------===//

// The lexer returns tokens [0-255] if it is an unknown character, otherwise one
// of these for known things.
enum Token {
  tok_eof = -1,

  // commands
  tok_func = -2,
  tok_extern = -3,

  // primary
  tok_identifier = -4,
  tok_number = -5,

  // control
  tok_if = -6,
  tok_else = -8,
  tok_for = -9,
  tok_while = -10,
  tok_return = -11,

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

      while (isalnum((LastChar = getchar())) || LastChar == '_')
        IdentifierStr += (char)LastChar;

      if (IdentifierStr == "func")
        return tok_func;
      if (IdentifierStr == "extern")
        return tok_extern;
      if (IdentifierStr == "if")
        return tok_if;
      if (IdentifierStr == "else")
        return tok_else;
      if (IdentifierStr == "for")
        return tok_for;
      if (IdentifierStr == "while")
        return tok_while;
      if (IdentifierStr == "return")
        return tok_return;
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
  Parser(FILE *Stream, std::string Path) : L(Stream), Path(std::move(Path)) {
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
      case tok_func: HandleDefinition(Unit); break;
      case tok_extern: HandleExtern(Unit); break;
      default: assert(false && "unexpected token"); break;
      }
    }
  }

private:
  Lexer L;

  std::string Path;

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
    case tok_eof: return "<tok_eof>";
    case tok_func: return "<tok_func>";
    case tok_extern: return "<tok_extern>";
    case tok_identifier: return "<tok_identifier>";
    case tok_number: return "<tok_number>";
    case tok_if: return "<tok_if>";
    case tok_else: return "<tok_else>";
    case tok_for: return "<tok_for>";
    case tok_while: return "<tok_while>";
    case tok_var: return "<tok_var>";
    default: return "<unknown>";
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

  std::vector<std::unique_ptr<StmtAST>> ParseStmts() {
    std::vector<std::unique_ptr<StmtAST>> Stmts;

    while (CurTok != '}') {
      auto S = ParseStmt();
      if (!S)
        continue;
      Stmts.emplace_back(std::move(S));
    }

    return Stmts;
  }

  /// stmt
  ///   ::= block
  ///   ::= ifstmt
  ///   ::= whilestmt
  ///   ::= varstmt
  ///   ::= expr ';'
  ///   ::= ';'
  std::unique_ptr<StmtAST> ParseStmt() {
    if (CurTok == '{')
      return ParseBlockStmt();

    if (CurTok == tok_if)
      return ParseIfStmt();

    if (CurTok == tok_while)
      return ParseWhileStmt();

    if (CurTok == tok_var)
      return ParseVarStmt();

    if (CurTok == tok_return)
      return ParseReturnStmt();

    if (CurTok == ';') {
      getNextToken();
      return nullptr;
    }

    auto E = ParseExpression();
    if (!E)
      return nullptr;
    if (CurTok != ';')
      return logError(Expected({';'}), After("expression"));
    getNextToken();

    return std::make_unique<ExprStmtAST>(std::move(E));
  }

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

  /// block ::= '{' stmts '}'
  std::unique_ptr<StmtAST> ParseBlockStmt() {
    getNextToken(); // eat '{'.
    auto Block = ParseStmts();

    if (CurTok != '}')
      return logError(Expected({'}'}), In("block"));

    getNextToken(); // eat '}'.
    return std::make_unique<BlockStmtAST>(std::move(Block));
  }

  /// ifexpr
  ///   ::= 'if' expr block
  ///   ::= 'if' expr block else block
  std::unique_ptr<StmtAST> ParseIfStmt() {
    getNextToken(); // eat the if.

    // condition.
    auto Cond = ParseExpression();
    if (!Cond)
      return nullptr;

    if (CurTok != '{')
      return logError(Expected({'{'}), In("if statement"));

    auto Then = ParseBlockStmt();
    if (!Then)
      return nullptr;

    if (CurTok != tok_else)
      return std::make_unique<IfStmtAST>(std::move(Cond), std::move(Then));

    getNextToken(); // eat 'else'.
    if (CurTok != '{') {
      return logError(Expected({'{'}), In("if statement"));
    }

    auto Else = ParseBlockStmt();
    if (!Else)
      return nullptr;

    return std::make_unique<IfStmtAST>(std::move(Cond), std::move(Then),
                                       std::move(Else));
  }

  /// whilestmt ::= 'while' expr block
  std::unique_ptr<StmtAST> ParseWhileStmt() {
    getNextToken(); // eat 'while'.

    auto Cond = ParseExpression();
    if (!Cond)
      return nullptr;

    if (CurTok != '{') {
      return logError(Expected({'{'}), In("while statement"));
    }

    auto Body = ParseBlockStmt();
    if (!Body)
      return nullptr;

    return std::make_unique<WhileStmtAST>(std::move(Cond), std::move(Body));
  }

  /// varstmt ::= 'var' identifier ':' identifier ('=' expr)? ';'
  std::unique_ptr<StmtAST> ParseVarStmt() {
    getNextToken(); // eat the var.

    // At least one variable name is required.
    if (CurTok != tok_identifier)
      return logError(Expected({tok_identifier}), After("var"));

    std::string Name = L.getIdentifierStr();
    getNextToken(); // eat identifier.

    if (CurTok != ':')
      return logError(Expected({':'}), After("identifier"));
    getNextToken(); // eat ':'.

    if (CurTok != tok_identifier)
      return logError(Expected({tok_identifier}), After(":"));

    std::string Type = L.getIdentifierStr();
    getNextToken(); // eat identifier.

    // Read the optional initializer.
    std::unique_ptr<ExprAST> Init = nullptr;
    if (CurTok == '=') {
      getNextToken(); // eat the '='.

      Init = ParseExpression();
      if (!Init)
        return nullptr;
    }

    if (CurTok != ';')
      return logError(Expected({';'}), In("var statement"));

    return std::make_unique<VarStmtAST>(std::move(Name), std::move(Type),
                                        std::move(Init));
  }

  /// returnstmt ::= 'return' expr? ';'
  std::unique_ptr<StmtAST> ParseReturnStmt() {
    getNextToken(); // eat 'return'.

    if (CurTok == ';')
      return std::make_unique<ReturnStmtAST>();

    auto E = ParseExpression();
    if (!E)
      return nullptr;

    if (CurTok != ';')
      return logError(Expected({';'}), After("Expression"));
    getNextToken();

    return std::make_unique<ReturnStmtAST>(std::move(E));
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
    default:
      return logError(Expected({tok_identifier, tok_number, '('}),
                      In("primary"));
    case tok_identifier: return ParseIdentifierExpr();
    case tok_number: return ParseNumberExpr();
    case '(': return ParseParenExpr();
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
  std::unique_ptr<ExprAST> ParseExpression() {
    auto LHS = ParseUnary();
    if (!LHS)
      return nullptr;

    return ParseBinOpRHS(0, std::move(LHS));
  }

  /// prototype ::= identifier '(' (param (',' param)*)? ')' ( ':' identifier)?
  std::unique_ptr<PrototypeAST> ParsePrototype() {
    std::string FnName;

    if (CurTok != tok_identifier)
      return logError(Expected({tok_identifier}), In("prototype"));

    FnName = L.getIdentifierStr();
    getNextToken(); // eat identifier

    if (CurTok != '(') {
      return logError(Expected({'('}), In("prototype"));
    }
    getNextToken(); // eat '('.

    if (CurTok != tok_identifier && CurTok != ')')
      return logError(Expected({')', tok_identifier}), In("parameter list"));

    std::vector<std::pair<std::string, std::string>> Params;
    while (CurTok == tok_identifier) {
      auto Name = L.getIdentifierStr();
      if (getNextToken() != ':')
        return logError(Expected({':'}), After("parameter name"));
      if (getNextToken() != tok_identifier)
        return logError(Expected({tok_identifier}), After("':'"));
      auto Type = L.getIdentifierStr();
      Params.emplace_back(std::move(Name), std::move(Type));
      auto Next = getNextToken();
      if (Next == ')')
        break;
      if (Next != ',')
        return logError(Expected({',', ')'}), After("parameter"));
      getNextToken();
    }
    if (CurTok != ')')
      return logError(Expected({')'}), In("prototype"));

    getNextToken(); // eat ')'.

    if (CurTok != ':')
      return std::make_unique<PrototypeAST>(FnName, Params);

    getNextToken(); // eat ':'.
    if (CurTok != tok_identifier)
      return logError(Expected({tok_identifier}), After("':'"));

    auto ReturnType = L.getIdentifierStr();
    getNextToken(); // eat identifier.
    return std::make_unique<PrototypeAST>(ReturnType, FnName, Params);
  }

  /// definition ::= 'func' prototype expression
  std::unique_ptr<FunctionAST> ParseDefinition() {
    getNextToken(); // eat 'func'.
    auto Proto = ParsePrototype();
    if (!Proto)
      return nullptr;

    if (auto E = ParseBlockStmt())
      return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    return nullptr;
  }

  /// external ::= 'extern' prototype
  std::unique_ptr<PrototypeAST> ParseExtern() {
    getNextToken(); // eat extern.
    return ParsePrototype();
  }

  void HandleDefinition(CompilationUnit &U) {
    if (auto FnAST = ParseDefinition())
      U.addFunction(std::move(FnAST));
    else
      // Skip token for error recovery.
      getNextToken();
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
  printError("{}:{}:{}: {}", Path, TokPos.first, TokPos.second, Str);
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
