#ifndef __AST_H__
#define __AST_H__

#include "../token/token.h"

typedef struct Statement
{
  char *token_literal;
} Statement;

typedef struct Expression
{
  char *token_literal;
} Expression;

typedef struct StatementListNode
{
  Statement *node;
  Statement *next;

} StatementListNode;

typedef struct Program
{
  StatementListNode *statements;
} Program;

typedef struct Identifier
{
  Token *token;
  char *value;
} Identifier;

typedef struct LetStatement
{
  Token *token;
  Identifier *name;
  Expression *value;
} LetStatement;

#endif // __AST_H__
