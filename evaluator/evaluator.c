#include "evaluator.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ast/ast.h"
#include "../parser/parser.h"
#include "../utils/list.h"

Object M_NULL = {NULL_OBJ, {0}};
Object TRUE = {BOOLEAN_OBJ, {.b = true}};
Object FALSE = {BOOLEAN_OBJ, {.b = false}};

Object eval_prefix_expression(char *operator, Object right);
Object eval_statements(List *statements);

Object eval(void *node, NodeType type) {
  Object object = {INTEGER_OBJ, {0}};
  switch (type) {
    case PROGRAM_NODE:
      return eval_statements(((Program *)node)->statements);
    case EXPRESSION_STATEMENT_NODE:
      return eval(((ExpressionStatement *)node)->expression, EXPRESSION_NODE);
    case INTEGER_LITERAL_NODE:
      object.type = INTEGER_OBJ;
      object.value.i = ((IntegerLiteral *)node)->value;
      return object;
    case BOOLEAN_LITERAL_NODE:
      return ((BooleanLiteral *)node)->value ? TRUE : FALSE;
    case EXPRESSION_NODE: {
      Expression *exp = node;
      switch (exp->type) {
        case EXPRESSION_PREFIX: {
          PrefixExpression *pfx = ((PrefixExpression *)exp->node);
          Object right = eval(pfx->right, EXPRESSION_NODE);
          return eval_prefix_expression(pfx->operator, right);
        }
        case EXPRESSION_BOOLEAN_LITERAL:
          return eval(exp->node, BOOLEAN_LITERAL_NODE);
        case EXPRESSION_INTEGER_LITERAL:
          return eval(exp->node, INTEGER_LITERAL_NODE);
      }
    }
  }
  return object;
}

Object eval_statements(List *statements) {
  Object object;
  List *current = statements;
  for (; current != NULL; current = current->next)
    if (current->item != NULL) {
      int type = -1;
      Statement *stmt = (Statement *)current->item;
      switch (stmt->type) {
        case STATEMENT_RETURN:
          type = 99;  // TODO
        case STATEMENT_LET:
          type = 88;  // TODO
        case STATEMENT_EXPRESSION:
          type = EXPRESSION_STATEMENT_NODE;
      }
      object = eval(stmt->node, type);
    }
  return object;
}

Object eval_bang_operator_expression(Object right) {
  if (right.type == BOOLEAN_OBJ) {
    if (right.value.b == true)
      return FALSE;
    else
      return TRUE;
  } else if (right.type == NULL_OBJ) {
    return TRUE;
  } else {
    return FALSE;
  }
}

Object eval_minus_prefix_operator_expression(Object right) {
  if (right.type != INTEGER_OBJ) {
    return M_NULL;
  }
  int value = right.value.i;
  Object object = {INTEGER_OBJ, {.i = value * -1}};
  return object;
}

Object eval_prefix_expression(char *operator, Object right) {
  if (strcmp(operator, "!") == 0)
    return eval_bang_operator_expression(right);
  else if (strcmp(operator, "-") == 0)
    return eval_minus_prefix_operator_expression(right);
  else
    return M_NULL;
}
