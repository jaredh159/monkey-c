#ifndef __AST_H__
#define __AST_H__

#include "../token/token.h"

typedef struct Expression
{
  char *token_literal;
} Expression;

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

typedef struct ReturnStatement
{
  Token *token;
  Expression *return_value;
} ReturnStatement;

typedef struct StatementType
{
  unsigned int is_statement : 1;
  unsigned int is_expression : 1;
} StatementType;

typedef struct Statement
{
  char *token_literal;
  StatementType type;
  LetStatement *let_statement;
  ReturnStatement *return_statement;
} Statement;

typedef struct Statements
{
  Statement *statement;
  struct Statements *next;
} Statements;

typedef struct Program
{
  char *token_literal;
  Statements *statements;
} Program;

void print_program(Program *program);
void print_statement(Statement *statement);
void print_expression(Expression *expression);
void print_identifier(Identifier *identifier);
int num_program_statements(Program *program);

#endif // __AST_H__
