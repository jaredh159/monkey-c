#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdbool.h>
#include "../ast/ast.h"

enum Precedence {
  PRECEDENCE_LOWEST,
  PRECEDENCE_EQUALS,
  PRECEDENCE_LESSGREATER,
  PRECEDENCE_SUM,
  PRECEDENCE_PRODUCT,
  PRECEDENCE_PREFIX,
  PRECEDENCE_CALL,
  PRECEDENCE_INDEX,
};

enum ExpressionType {
  EXPRESSION_IDENTIFIER,
  EXPRESSION_INTEGER_LITERAL,
  EXPRESSION_BOOLEAN_LITERAL,
  EXPRESSION_FUNCTION_LITERAL,
  EXPRESSION_STRING_LITERAL,
  EXPRESSION_ARRAY_LITERAL,
  EXPRESSION_HASH_LITERAL,
  EXPRESSION_INDEX,
  EXPRESSION_PREFIX,
  EXPRESSION_INFIX,
  EXPRESSION_IF,
  EXPRESSION_CALL
};

enum StatementType { STATEMENT_LET, STATEMENT_RETURN, STATEMENT_EXPRESSION };

Program *parse_program(char *input);
Expression *parse_expression(int precedence);
BlockStatement *parse_block_statement();
List *parse_function_parameters();
List *parse_expression_list(int end_token_type);
void parser_next_token();
void parser_push_error(char *error_msg);
int parser_current_precedence();
int parser_peek_precedence();
bool parser_has_error();
int parser_num_errors();
void parser_print_errors();
Token *parser_current_token();
Token *parser_peek_token();
bool parser_peek_token_is(int token_type);
bool parser_current_token_is(int token_type);
bool parser_expect_peek(int token_type);
typedef Expression *(*PrefixParselet)(void);
typedef Expression *(*InfixParselet)(Expression *);
PrefixParselet get_prefix_parselet(int token_type);
InfixParselet get_infix_parselet(int token_type);

#endif  // __PARSER_H__
