// Connor Melton CS280 Programming Assignment 2

// What im doing:
// Making a parser. It is an algorithm designed in order to take a sequence of
// tokens and decide whether that sequence is valid in the considered grammar.

// Usually just follow the grammar. “one function per grammar rule.”
// while (true) GetNextToken() is how to interpret EBNF {...} meaning zero or
// more

// Recursion occurs when a nonterminal (function) eventually calls itself,
// directly or indirectly, through the grammar structure.
//
// Example:
//
// StmtList ::= Stmt ; { Stmt ; }
// Stmt     ::= IfStmt | AssignStmt | PrintLnStmt
// IfStmt   ::= IF (Expr) '{' StmtList '}' [ ELSE '{' StmtList '}' ]
//
// StmtList calls Stmt.
// If the statement is an IfStmt, then IfStmt calls StmtList again.
// This creates an indirect recursive cycle:
//     StmtList → Stmt → IfStmt → StmtList
//
// That cycle is where the recursion comes from

// How to code this: Go in order of grammar. Implement logic without stepping
// into recursive cases or subcalls. Just write the method and grammar checks,
// only then try writing code for checks for subgrammar rules.
// ^^^^OTHERWISE FORGET WHAT I WAS DOING

#include "parser.h"

#include <iterator>

map<string, bool> defVar;
map<string, Token> SymTable;

namespace ErrMsg {
constexpr const char* MISSING_OPERAND = "Missing operand for an operator";
constexpr const char* MISSING_LP_PRINTLN =
    "Missing Left Parenthesis of PrintLn Statement";
constexpr const char* MISSING_RP_IF =
    "Missing Right Parenthesis of If condition";
constexpr const char* MISSING_LBRACE_IF =
    "Missing left brace for If Statement Clause";
constexpr const char* ILLEGAL_ELSE = "Illegal If Statement Else-Clause";
constexpr const char* MISSING_RBRACE_ELSE =
    "Missing right brace for an Else-Clause";
constexpr const char* MISSING_ASSIGN = "Missing Assignment Operator";
constexpr const char* MISSING_SEMI = "Missing semicolon at end of Statement";
}  // namespace ErrMsg

namespace Parser {
bool pushed_back = false;
LexItem pushed_token;

static LexItem GetNextToken(istream& in, int& line) {
  if (pushed_back) {
    pushed_back = false;
    return pushed_token;
  }
  return getNextToken(in, line);
}

static void PushBackToken(LexItem& t) {
  if (pushed_back) {
    abort();
  }
  pushed_back = true;
  pushed_token = t;
}

static void successMsgBuilder() {
  cout << "Declared Variables:";
  for (auto iter = SymTable.begin(); iter != SymTable.end(); iter++) {
    if (next(iter) != SymTable.end()) {
      cout << iter->second << ", ";
    } else {
      cout << iter->second;
    }
  }
  cout << DONE << "\n"
       << "Successful Parsing" << endl;
}

// -------------------------------Parserlogic-----------------------------------------
static bool Prog(istream& in, int& line) { return StmtList(in, line); }

// StmtList ::= Stmt ; { Stmt ; }
static bool StmtList(istream& in, int& line) {
  // Stmt;
  if (!Stmt(in, line)) {  // first Stmt required
    return false;
  }

  LexItem tok = GetNextToken(in, line);

  if (tok.GetToken() == DONE) {
    successMsgBuilder();
    return true;  // Empty program valid
  }
  if (tok.GetToken() == ERR) {
    ParseError(line, "Lex ERR at StmtList()");
  }

  if (tok.GetToken() != SEMICOL) {
    ParseError(line, ErrMsg::MISSING_SEMI);
    return false;
  }

  // { Stmt; } means zero or more Stmt;
  while (true) {
    tok = GetNextToken(in, line);

    // zero
    bool isStmt = tok.GetToken() == IF || tok.GetToken() == IDENT ||
                  tok.GetToken() == PRINTLN;
    if (!isStmt) {
      PushBackToken(tok);
      return true;  // optional statement not present
    }
    // or more
    if (!Stmt(in, line)) {  // ensure valid stmt
      ParseError(line, "invalid statement");
      return false;
    }

    tok = GetNextToken(in, line);
    if (tok.GetToken() != SEMICOL) {
      ParseError(line, ErrMsg::MISSING_SEMI);
      return false;
    }
  }
}

// Stmt ::= IfStmt | AssignStmt | PrintLnStmt
static bool Stmt(istream& in, int& line) {
  LexItem tok = GetNextToken(in, line);

  if (tok.GetToken() == DONE) {  // Never DONE in middle of Stmt
    ParseError(line,
               "Cant end in middle of Stmt!");  // Keep it general
    return false;
  }
  if (tok.GetToken() == ERR) {
    ParseError(line, "lex err Error at Stmt()");
  }

  // IfStmt | AssignStmt | PrintLnStmt
  switch (tok.GetToken()) {
    case IF:
      IfStmt(in, line);
      break;
    case IDENT:
      AssignStmt(in, line);
      break;
    case PRINTLN:
      PrintLnStmt(in, line);
      break;
    default:
      ParseError(line, "not a valid statement at stmt()");
  }
}

static bool PrintLnStmt(istream& in, int& line) {
  LexItem tok = GetNextToken(in, line);
  if (tok.GetToken() == DONE) {
    successMsgBuilder();
    return true;
  }
  if (tok.GetToken() == ERR) {
    ParseError(line, "lex err");
  }
}

// IfStmt ::= IF (Expr) ‘{‘ StmtList ‘}’ [ ELSE ‘{‘ StmtList ‘}’ ]
static bool IfStmt(istream& in, int& line) {
  LexItem tok = GetNextToken(in, line);

  if (tok.GetToken() == DONE) {  // Never DONE in middle of IfStmt
    ParseError(line, "Cant end in middle of IfStmt!");
    return false;
  }
  if (tok.GetToken() == ERR) {
    ParseError(line, "lex err Error at IfStmt()");
  }

  if (tok.GetToken() == IF) {
    tok = GetNextToken(in, line);
    if (tok.GetToken() == RPAREN) {
      tok = GetNextToken(in, line);
    }
  }
}

static bool AssignStmt(istream& in, int& line) {
  if (tok.GetToken() == DONE) {
    successMsgBuilder();
    return true;
  }
  if (tok.GetToken() == ERR) {
    ParseError(line, ErrMsg::MISSING_OPERAND);
  }
}

static bool Var(istream& in, int& line) {
  if (tok.GetToken() == DONE) {
    successMsgBuilder();
    return true;
  }
  if (tok.GetToken() == ERR) {
    ParseError(line, ErrMsg::MISSING_OPERAND);
  }
}

static bool ExprList(istream& in, int& line) {
  if (tok.GetToken() == DONE) {
    successMsgBuilder();
    return true;
  }
  if (tok.GetToken() == ERR) {
    ParseError(line, ErrMsg::MISSING_OPERAND);
  }
}

static bool Expr(istream& in, int& line) {
  if (tok.GetToken() == DONE) {
    successMsgBuilder();
    return true;
  }
  if (tok.GetToken() == ERR) {
    ParseError(line, ErrMsg::MISSING_OPERAND);
  }
}

static bool AssigOp(istream& in, int& line) {
  if (tok.GetToken() == DONE) {
    successMsgBuilder();
    return true;
  }
  if (tok.GetToken() == ERR) {
    ParseError(line, ErrMsg::MISSING_OPERAND);
  }
}

static bool OrExpr(istream& in, int& line) {
  if (tok.GetToken() == DONE) {
    successMsgBuilder();
    return true;
  }
  if (tok.GetToken() == ERR) {
    ParseError(line, ErrMsg::MISSING_OPERAND);
  }
}

static bool AndExpr(istream& in, int& line) {
  if (tok.GetToken() == DONE) {
    successMsgBuilder();
    return true;
  }
  if (tok.GetToken() == ERR) {
    ParseError(line, ErrMsg::MISSING_OPERAND);
  }
}

static bool RelExpr(istream& in, int& line) {
  if (tok.GetToken() == DONE) {
    successMsgBuilder();
    return true;
  }
  if (tok.GetToken() == ERR) {
    ParseError(line, ErrMsg::MISSING_OPERAND);
  }
}

static bool AddExpr(istream& in, int& line) {
  if (tok.GetToken() == DONE) {
    successMsgBuilder();
    return true;
  }
  if (tok.GetToken() == ERR) {
    ParseError(line, ErrMsg::MISSING_OPERAND);
  }
}

static bool MultExpr(istream& in, int& line) {
  if (tok.GetToken() == DONE) {
    successMsgBuilder();
    return true;
  }
  if (tok.GetToken() == ERR) {
    ParseError(line, ErrMsg::MISSING_OPERAND);
  }
}

static bool UnaryExpr(istream& in, int& line) {
  if (tok.GetToken() == DONE) {
    successMsgBuilder();
    return true;
  }
  if (tok.GetToken() == ERR) {
    ParseError(line, ErrMsg::MISSING_OPERAND);
  }
}

static bool ExponExpr(istream& in, int& line, int sign) {
  if (tok.GetToken() == DONE) {
    successMsgBuilder();
    return true;
  }
  if (tok.GetToken() == ERR) {
    ParseError(line, ErrMsg::MISSING_OPERAND);
  }
}

static bool PrimaryExpr(istream& in, int& line, int sign) {
  if (tok.GetToken() == DONE) {
    successMsgBuilder();
    return true;
  }
  if (tok.GetToken() == ERR) {
    ParseError(line, ErrMsg::MISSING_OPERAND);
  }
}
}  // namespace Parser

static int error_count = 0;

int ErrCount() { return error_count; }

void ParseError(int line, string msg) {
  ++error_count;
  cout << error_count << ". Line " << line << ": " << msg << endl;
}
