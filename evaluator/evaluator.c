#include "evaluator.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "../ast/ast.h"
#include "../parser/parser.h"
#include "../utils/list.h"

Object M_NULL = {NULL_OBJ, {0}};
Object TRUE = {BOOLEAN_OBJ, {.b = true}};
Object FALSE = {BOOLEAN_OBJ, {.b = false}};

Object eval_integer_infix_expression(char *operator, Object left, Object right);
Object eval_infix_expression(char *operator, Object left, Object right);
Object eval_prefix_expression(char *operator, Object right);
Object eval_if_expression(IfExpression *if_exp);
Object eval_program(List *statements);
Object eval_block_statement(List *statements);

Object eval(void *node, NodeType type) {
  Object object = {INTEGER_OBJ, {0}};
  switch (type) {
    case PROGRAM_NODE:
      return eval_program(((Program *)node)->statements);
    case BLOCK_STATEMENTS_NODE:
      return eval_block_statement(((BlockStatement *)node)->statements);
    case RETURN_STATEMENT_NODE: {
      Object temp =
        eval(((ReturnStatement *)node)->return_value, EXPRESSION_NODE);
      // can't return address of something on the stack, so need to malloc here
      Object *return_value = malloc(sizeof(Object));
      return_value->type = temp.type;
      return_value->value = temp.value;
      object.type = RETURN_VALUE_OBJ;
      object.value.return_value = return_value;
      return object;
    }
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
        case EXPRESSION_INFIX: {
          InfixExpression *infix = ((InfixExpression *)exp->node);
          Object left = eval(infix->left, EXPRESSION_NODE);
          Object right = eval(infix->right, EXPRESSION_NODE);
          return eval_infix_expression(infix->operator, left, right);
        }
        case EXPRESSION_IF:
          return eval_if_expression(((IfExpression *)exp->node));
        case EXPRESSION_BOOLEAN_LITERAL:
          return eval(exp->node, BOOLEAN_LITERAL_NODE);
        case EXPRESSION_INTEGER_LITERAL:
          return eval(exp->node, INTEGER_LITERAL_NODE);
      }
    }
  }
  return object;
}

Object eval_statement(Statement *stmt) {
  int type = -1;
  switch (stmt->type) {
    case STATEMENT_RETURN:
      type = RETURN_STATEMENT_NODE;
      break;
    case STATEMENT_LET:
      type = 88;  // TODO
      break;
    case STATEMENT_EXPRESSION:
      type = EXPRESSION_STATEMENT_NODE;
      break;
  }
  return eval(stmt->node, type);
}

Object eval_program(List *statements) {
  Object object;
  List *current = statements;
  for (; current != NULL; current = current->next)
    if (current->item != NULL) {
      Statement *stmt = (Statement *)current->item;
      object = eval_statement(stmt);
      if (object.type == RETURN_VALUE_OBJ) {
        return *object.value.return_value;
      }
    }
  return object;
}

Object eval_block_statement(List *statements) {
  Object object;
  List *current = statements;
  for (; current != NULL; current = current->next)
    if (current->item != NULL) {
      Statement *stmt = (Statement *)current->item;
      object = eval_statement(stmt);
      if (object.type == RETURN_VALUE_OBJ) {
        return object;
      }
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
  switch (*operator) {
    case '!':
      return eval_bang_operator_expression(right);
    case '-':
      return eval_minus_prefix_operator_expression(right);
    default:
      return M_NULL;
  }
}

Object eval_infix_expression(char *operator, Object left, Object right) {
  if (left.type == INTEGER_OBJ && right.type == INTEGER_OBJ) {
    return eval_integer_infix_expression(operator, left, right);
  }
  switch (*operator) {
    case '=':  // `==`
      return left.value.b == right.value.b ? TRUE : FALSE;
    case '!':  // `!=`
      return left.value.b != right.value.b ? TRUE : FALSE;
  }
  return M_NULL;
}

Object eval_integer_infix_expression(
  char *operator, Object left, Object right) {
  Object object = {INTEGER_OBJ, {.i = 0}};
  switch (*operator) {
    case '+':
      object.value.i = left.value.i + right.value.i;
      return object;
    case '-':
      object.value.i = left.value.i - right.value.i;
      return object;
    case '*':
      object.value.i = left.value.i * right.value.i;
      return object;
    case '/':
      object.value.i = left.value.i / right.value.i;
      return object;
    case '<':
      return left.value.i < right.value.i ? TRUE : FALSE;
    case '>':
      return left.value.i > right.value.i ? TRUE : FALSE;
    case '=':  // `==`
      return left.value.i == right.value.i ? TRUE : FALSE;
    case '!':  // `!=
      return left.value.i != right.value.i ? TRUE : FALSE;
    default:
      return M_NULL;
  }
}

bool is_truthy(Object obj) {
  if (obj.type == NULL_OBJ)
    return false;
  else if (obj.type == BOOLEAN_OBJ && obj.value.b == false)
    return false;
  else
    return true;
}

Object eval_if_expression(IfExpression *if_exp) {
  Object condition = eval(if_exp->condition, EXPRESSION_NODE);
  if (is_truthy(condition))
    return eval(if_exp->consequence, BLOCK_STATEMENTS_NODE);
  else if (if_exp->alternative != NULL)
    return eval(if_exp->alternative, BLOCK_STATEMENTS_NODE);
  else
    return M_NULL;
}
