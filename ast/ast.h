#ifndef __AST_H__
#define __AST_H__

#include <stdbool.h>
#include "../token/token.h"
#include "../utils/list.h"

enum NodeTypes {
  PROGRAM_NODE,
  EXPRESSION_NODE,
  EXPRESSION_STATEMENT_NODE,
  RETURN_STATEMENT_NODE,
  LET_STATEMENT_NODE,
  INTEGER_LITERAL_NODE,
  BOOLEAN_LITERAL_NODE,
  BLOCK_STATEMENTS_NODE,
};

typedef int NodeType;

typedef struct StringLiteral {
  Token *token;
  char *value;
} StringLiteral;

typedef struct BooleanLiteral {
  Token *token;
  bool value;
} BooleanLiteral;

typedef struct ArrayLiteral {
  Token *token;
  List *elements;
} ArrayLiteral;

typedef struct IntegerLiteral {
  Token *token;
  int value;
} IntegerLiteral;

typedef struct Expression {
  char *token_literal;
  int type;
  void *node;
} Expression;

typedef struct HashLiteralPair {
  Expression *key;
  Expression *value;
} HashLiteralPair;

typedef struct HashLiteralExpression {
  Token *token;
  List *pairs;  // List<HashLiteralPair>
} HashLiteralExpression;

typedef struct InfixExpression {
  Token *token;
  char *operator;
  Expression *right;
  Expression *left;
} InfixExpression;

typedef struct IndexExpression {
  Token *token;
  Expression *left;
  Expression *index;
} IndexExpression;

typedef struct PrefixExpression {
  Token *token;
  char *operator;
  Expression *right;
} PrefixExpression;

typedef struct Identifier {
  Token *token;
  char *value;
} Identifier;

typedef struct LetStatement {
  Token *token;
  Identifier *name;
  Expression *value;
} LetStatement;

typedef struct ReturnStatement {
  Token *token;
  Expression *return_value;
} ReturnStatement;

typedef struct ExpressionStatement {
  Token *token;
  Expression *expression;
} ExpressionStatement;

typedef struct Statement {
  char *token_literal;
  int type;
  void *node;
} Statement;

typedef struct BlockStatement {
  Token *token;
  List *statements;
} BlockStatement;

typedef struct IfExpression {
  Token *token;
  Expression *condition;
  BlockStatement *consequence;
  BlockStatement *alternative;
} IfExpression;

typedef struct FunctionLiteral {
  Token *token;
  List *parameters;
  BlockStatement *body;
  char *name;
} FunctionLiteral;

typedef struct CallExpression {
  Token *token;    // the `(` token
  Expression *fn;  // Identifier or FunctionLiteral
  List *arguments;
} CallExpression;

typedef struct Program {
  char *token_literal;
  List *statements;
} Program;

NodeType ast_statement_node_type(Statement *statement);
char *program_string(Program *program);
char *statement_string(Statement *statement);
char *let_statement_string(LetStatement *let_statement);
char *return_statement_string(ReturnStatement *return_statement);
char *expression_statement_string(ExpressionStatement *expression_statement);
char *function_literal_expression_string(FunctionLiteral *fn);
char *call_expression_string(CallExpression *ce);
char *identifier_string(Identifier *identifier);
char *function_params_string(List *params);
char *block_statement_string(BlockStatement *bs);
char *string_literal_string(StringLiteral *string);
char *array_literal_string(ArrayLiteral *array_literal);
char *hash_literal_string(HashLiteralExpression *hash_literal);
char *index_expression_string(IndexExpression *index);
int token_precedence(int token_type);
ReturnStatement *get_return(Statement *statement);
LetStatement *get_let(Statement *statement);
ExpressionStatement *get_expression(Statement *statement);

#endif  // __AST_H__
