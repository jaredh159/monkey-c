#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdbool.h>
#include "../ast/ast.h"

enum
{
  PRECEDENCE_LOWEST,
  PRECEDENCE_EQUALS,
  PRECEDENCE_LESSGREATER,
  PRECEDENCE_SUM,
  PRECEDENCE_PRODUCT,
  PRECEDENCE_PREFIX,
  PRECEDENCE_CALL
};

enum
{
  EXPRESSION_IDENTIFIER,
  EXPRESSION_INTEGER_LITERAL,
  EXPRESSION_PREFIX
};

enum
{
  STATEMENT_LET,
  STATEMENT_RETURN,
  STATEMENT_EXPRESSION
};

Program *parse_program(char *input);
Expression *parse_expression(/* int precedence */);
void parser_next_token();
void parser_push_error(char *error_msg);
bool parser_has_error();
int parser_num_errors();
void parser_print_errors();
Token *parser_current_token();
Token *parser_peek_token();
typedef Expression *(*PrefixParselet)(void);
typedef Expression *(*InfixParselet)(Expression *);
PrefixParselet get_prefix_parselet(char *token_type);

#endif // __PARSER_H__
