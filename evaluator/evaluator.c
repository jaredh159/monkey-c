#include "evaluator.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "../ast/ast.h"
#include "../parser/parser.h"
#include "../utils/list.h"

Object M_NULL = {NULL_OBJ};
Object TRUE = {BOOLEAN_OBJ, {.b = true}};
Object FALSE = {BOOLEAN_OBJ, {.b = false}};

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
