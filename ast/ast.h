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

typedef struct ExpressionStatement
{
  Token *token;
  Expression *expression;
} ExpressionStatement;

typedef struct StatementType
{
  unsigned int is_let : 1;
  unsigned int is_return : 1;
  unsigned int is_expression : 1;
} StatementType;

typedef struct Statement
{
  char *token_literal;
  StatementType type;
  LetStatement *let_statement;
  ReturnStatement *return_statement;
  ExpressionStatement *expression_statement;
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

char *program_string(Program *program);
char *statement_string(Statement *statement);
char *let_statement_string(LetStatement *let_statement);
char *return_statement_string(ReturnStatement *return_statement);
char *expression_statement_string(ExpressionStatement *expression_statement);
char *identifier_string(Identifier *identifier);
void print_program(Program *program);
void print_statement(Statement *statement);
void print_expression(Expression *expression);
void print_identifier(Identifier *identifier);
int num_program_statements(Program *program);

#endif // __AST_H__
