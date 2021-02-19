#include "ast.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../parser/parser.h"
#include "../utils/colors.h"

static char *expression_string(Expression *exp);

void statement_invariant(
  Statement *statement, bool type_predicate, char *type) {
  if (type_predicate)
    return;
  printf(COLOR_RED "statement not of required type `%s`\n" COLOR_RESET, type);
  printf("statement: %s", statement_string(statement));
  exit(1);
}

ReturnStatement *get_return(Statement *statement) {
  statement_invariant(statement, statement->type == STATEMENT_RETURN, "return");
  return (ReturnStatement *)statement->node;
}

LetStatement *get_let(Statement *statement) {
  statement_invariant(statement, statement->type == STATEMENT_LET, "let");
  return (LetStatement *)statement->node;
}

ExpressionStatement *get_expression(Statement *statement) {
  statement_invariant(
    statement, statement->type == STATEMENT_EXPRESSION, "expression");
  return (ExpressionStatement *)statement->node;
}

#define MAX_STMT_STR_LEN 100

char *program_string(Program *program) {
  int num_stmts = list_count(program->statements);
  char *prog_str = malloc(MAX_STMT_STR_LEN * num_stmts);
  prog_str[0] = '\0';
  if (num_stmts == 0)
    return prog_str;

  list_strcat_each(program->statements, prog_str, (StrHandler)statement_string);
  return prog_str;
}

char *statement_string(Statement *statement) {
  if (statement->type == STATEMENT_LET)
    return let_statement_string(statement->node);
  else if (statement->type == STATEMENT_RETURN)
    return return_statement_string(statement->node);
  else
    return expression_statement_string(statement->node);
}

char *let_statement_string(LetStatement *ls) {
  char *let_str = malloc(MAX_STMT_STR_LEN);
  sprintf(let_str, "%s %s = %s;\n", ls->token->literal, ls->name->value,
    expression_string(ls->value));
  return let_str;
}

char *return_statement_string(ReturnStatement *rs) {
  char *ret_str = malloc(MAX_STMT_STR_LEN);
  sprintf(ret_str, "%s %s;\n", rs->token->literal,
    expression_string(rs->return_value));
  return ret_str;
}

char *string_literal_string(StringLiteral *string) {
  return string->token->literal;
}

char *integer_literal_string(IntegerLiteral *int_literal) {
  return int_literal->token->literal;
}

char *identifier_string(Identifier *identifier) {
  return identifier->value;
}

char *infix_expression_string(InfixExpression *infix) {
  char *inf_str = malloc(MAX_STMT_STR_LEN);
  sprintf(inf_str, "(%s %s %s)", expression_string(infix->left),
    infix->operator, expression_string(infix->right));
  return inf_str;
}

char *prefix_expression_string(PrefixExpression *prefix) {
  char *pref_str = malloc(MAX_STMT_STR_LEN);
  sprintf(
    pref_str, "(%s%s)", prefix->operator, expression_string(prefix->right));
  return pref_str;
}

char *block_statement_string(BlockStatement *bs) {
  int num_stmts = list_count(bs->statements);
  char *bs_str = malloc(MAX_STMT_STR_LEN * num_stmts);
  bs_str[0] = '\0';
  if (num_stmts == 0)
    return bs_str;

  list_strcat_each(bs->statements, bs_str, (StrHandler)statement_string);

  return bs_str;
}

char *if_expression_string(IfExpression *if_exp) {
  char *if_str = malloc(MAX_STMT_STR_LEN);
  sprintf(if_str, "if %s %s%s", expression_string(if_exp->condition),
    block_statement_string(if_exp->consequence),
    if_exp->alternative == NULL ? ""
                                : block_statement_string(if_exp->alternative));
  return if_str;
}

char *array_literal_string(ArrayLiteral *array_literal) {
  char *array_lit_str = malloc(MAX_STMT_STR_LEN);
  array_lit_str[0] = '[';
  array_lit_str[1] = '\0';
  list_str_join(array_literal->elements, ", ", array_lit_str,
    (StrHandler)expression_string);
  strcat(array_lit_str, "]");
  return array_lit_str;
}

char *function_params_string(List *params) {
  char *params_str = malloc(MAX_STMT_STR_LEN);
  params_str[0] = '\0';
  list_str_join(params, ", ", params_str, (StrHandler)identifier_string);
  return params_str;
}

char *function_literal_expression_string(FunctionLiteral *fn) {
  char *fn_str = malloc(MAX_STMT_STR_LEN);

  sprintf(fn_str, "%s(%s)%s", fn->token->literal,
    function_params_string(fn->parameters), block_statement_string(fn->body));

  return fn_str;
}

char *call_expression_string(CallExpression *ce) {
  char *ce_str = malloc(MAX_STMT_STR_LEN);
  ce_str[0] = '\0';

  char *args_str = malloc(MAX_STMT_STR_LEN);
  args_str[0] = '\0';
  list_str_join(ce->arguments, ", ", args_str, (StrHandler)expression_string);

  sprintf(ce_str, "%s(%s)", expression_string(ce->fn), args_str);

  return ce_str;
}

char *index_expression_string(IndexExpression *index) {
  char *ie_str = malloc(MAX_STMT_STR_LEN);
  sprintf(ie_str, "(%s[%s])", expression_string(index->left),
    expression_string(index->index));
  return ie_str;
}

char *hash_literal_pair_string(HashLiteralPair *pair) {
  char *pair_str = malloc(MAX_STMT_STR_LEN);
  sprintf(pair_str, "%s:%s", expression_string(pair->key),
    expression_string(pair->value));
  return pair_str;
}

char *hash_literal_string(HashLiteralExpression *hash_literal) {
  char *hl_str = malloc(MAX_STMT_STR_LEN);
  strcat(hl_str, "{");
  list_strcat_each(
    hash_literal->pairs, hl_str, (StrHandler)hash_literal_pair_string);
  strcat(hl_str, "}");
  return hl_str;
}

int token_precedence(int token_type) {
  switch (token_type) {
    case TOKEN_EQ:
      return PRECEDENCE_EQUALS;
    case TOKEN_NOT_EQ:
      return PRECEDENCE_EQUALS;
    case TOKEN_LT:
      return PRECEDENCE_LESSGREATER;
    case TOKEN_GT:
      return PRECEDENCE_LESSGREATER;
    case TOKEN_PLUS:
      return PRECEDENCE_SUM;
    case TOKEN_MINUS:
      return PRECEDENCE_SUM;
    case TOKEN_SLASH:
      return PRECEDENCE_PRODUCT;
    case TOKEN_ASTERISK:
      return PRECEDENCE_PRODUCT;
    case TOKEN_LEFT_PAREN:
      return PRECEDENCE_CALL;
    case TOKEN_LEFT_BRACKET:
      return PRECEDENCE_INDEX;
  }
  return PRECEDENCE_LOWEST;
}

static char *boolean_literal_string(BooleanLiteral *boolean) {
  return boolean->token->literal;
}

static char *expression_string(Expression *exp) {
  switch (exp->type) {
    case EXPRESSION_CALL:
      return call_expression_string(exp->node);
    case EXPRESSION_FUNCTION_LITERAL:
      return function_literal_expression_string(exp->node);
    case EXPRESSION_BOOLEAN_LITERAL:
      return boolean_literal_string(exp->node);
    case EXPRESSION_IDENTIFIER:
      return identifier_string(exp->node);
    case EXPRESSION_INTEGER_LITERAL:
      return integer_literal_string(exp->node);
    case EXPRESSION_PREFIX:
      return prefix_expression_string(exp->node);
    case EXPRESSION_INFIX:
      return infix_expression_string(exp->node);
    case EXPRESSION_ARRAY_LITERAL:
      return array_literal_string(exp->node);
    case EXPRESSION_INDEX:
      return index_expression_string(exp->node);
    case EXPRESSION_HASH_LITERAL:
      return hash_literal_string(exp->node);
  }
  return NULL;
}

char *expression_statement_string(ExpressionStatement *es) {
  return expression_string(es->expression);
}
