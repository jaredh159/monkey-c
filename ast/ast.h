#ifndef __AST_H__
#define __AST_H__

#include <stdbool.h>
#include "../token/token.h"
#include "../utils/list.h"

typedef struct BooleanLiteral
{
  Token *token;
  bool value;
} BooleanLiteral;

typedef struct IntegerLiteral
{
  Token *token;
  int value;
} IntegerLiteral;

typedef struct Expression
{
  char *token_literal;
  int type;
  void *node;
} Expression;

typedef struct InfixExpression
{
  Token *token;
  char *operator;
  Expression *right;
  Expression *left;
} InfixExpression;

typedef struct PrefixExpression
{
  Token *token;
  char *operator;
  Expression *right;
} PrefixExpression;

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

typedef struct Statement
{
  char *token_literal;
  int type;
  void *node;
} Statement;

typedef struct BlockStatement
{
  Token *token;
  List *statements;
} BlockStatement;

typedef struct IfExpression
{
  Token *token;
  Expression *condition;
  BlockStatement *consequence;
  BlockStatement *alternative;
} IfExpression;

typedef struct FunctionLiteral
{
  Token *token;
  List *parameters;
  BlockStatement *body;
} FunctionLiteral;

typedef struct Program
{
  char *token_literal;
  List *statements;
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
int token_precedence(int token_type);
ReturnStatement *get_return(Statement *statement);
LetStatement *get_let(Statement *statement);
ExpressionStatement *get_expression(Statement *statement);

#endif // __AST_H__
