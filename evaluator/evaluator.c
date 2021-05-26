#include "evaluator.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ast/ast.h"
#include "../object/object.h"
#include "../parser/parser.h"
#include "../utils/list.h"

Object eval_integer_infix_expression(char *operator, Object left, Object right);
Object eval_string_infix_expression(char *operator, Object left, Object right);
Object eval_infix_expression(char *operator, Object left, Object right);
Object eval_prefix_expression(char *operator, Object right);
Object eval_identifier(Identifier *ident, Env *env);
Object eval_if_expression(IfExpression *if_exp, Env *env);
Object eval_program(List *statements, Env *env);
Object eval_block_statement(List *statements, Env *env);
Object eval_index_expression(Object left, Object index);
Object eval_array_index_expression(Object array, Object index);
Object eval_hash_index_expression(Object hash, Object index);
Object eval_hash_literal(HashLiteralExpression *hash_lit_exp, Env *env);
List *eval_expressions(List *expressions, Env *env);
Object apply_function(Object fn, List *args);
Object unwrap_return_value(Object obj);
Env *extend_function_env(Function *fn, List *args);
Object error(char *fmt, char **types, int num_types);
bool is_error(Object object);

Object eval(void *node, NodeType type, Env *env) {
  Object object = {INTEGER_OBJ, {0}};
  switch (type) {
    case PROGRAM_NODE:
      return eval_program(((Program *)node)->statements, env);
    case BLOCK_STATEMENTS_NODE:
      return eval_block_statement(((BlockStatement *)node)->statements, env);
    case RETURN_STATEMENT_NODE: {
      Object wrapped =
        eval(((ReturnStatement *)node)->return_value, EXPRESSION_NODE, env);
      if (is_error(wrapped))
        return wrapped;
      object.type = RETURN_VALUE_OBJ;
      object.value.return_value = object_copy(wrapped);
      return object;
    }
    case LET_STATEMENT_NODE: {
      LetStatement *ls = (LetStatement *)node;
      object = eval(ls->value, EXPRESSION_NODE, env);
      if (is_error(object))
        return object;
      env_set(env, ls->name->value, object);
      return object;
    }
    case EXPRESSION_STATEMENT_NODE:
      return eval(
        ((ExpressionStatement *)node)->expression, EXPRESSION_NODE, env);
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
          PrefixExpression *pfx = exp->node;
          Object right = eval(pfx->right, EXPRESSION_NODE, env);
          if (is_error(right))
            return right;
          return eval_prefix_expression(pfx->operator, right);
        }
        case EXPRESSION_INFIX: {
          InfixExpression *infix = exp->node;
          Object left = eval(infix->left, EXPRESSION_NODE, env);
          if (is_error(left))
            return left;
          Object right = eval(infix->right, EXPRESSION_NODE, env);
          if (is_error(right))
            return right;
          return eval_infix_expression(infix->operator, left, right);
        }
        case EXPRESSION_STRING_LITERAL: {
          StringLiteral *str = exp->node;
          object.type = STRING_OBJ;
          object.value.str = str->value;
          return object;
        }
        case EXPRESSION_FUNCTION_LITERAL: {
          FunctionLiteral *fn = exp->node;
          object.type = FUNCTION_OBJ;
          object.value.fn = malloc(sizeof(Function));
          object.value.fn->parameters = fn->parameters;
          object.value.fn->body = fn->body;
          object.value.fn->env = env;
          return object;
        }
        case EXPRESSION_CALL: {
          CallExpression *call = exp->node;
          Object fn = eval(call->fn, EXPRESSION_NODE, env);
          if (is_error(fn))
            return fn;
          List *args = eval_expressions(call->arguments, env);
          if (list_count(args) > 0) {
            Object *first_arg = args->item;
            if (list_count(args) == 1 && is_error(*first_arg))
              return *first_arg;
          }
          return apply_function(fn, args);
        }
        case EXPRESSION_ARRAY_LITERAL: {
          ArrayLiteral *array = exp->node;
          List *elements = eval_expressions(array->elements, env);
          if (list_count(elements) > 0) {
            Object *first_el = elements->item;
            if (list_count(elements) == 1 && is_error(*first_el))
              return *first_el;
          }
          object.type = ARRAY_OBJ;
          object.value.list = elements;
          return object;
        }
        case EXPRESSION_INDEX: {
          IndexExpression *ie = exp->node;
          Object left = eval(ie->left, EXPRESSION_NODE, env);
          if (is_error(left))
            return left;
          Object index = eval(ie->index, EXPRESSION_NODE, env);
          if (is_error(index))
            return index;
          return eval_index_expression(left, index);
        }
        case EXPRESSION_HASH_LITERAL:
          return eval_hash_literal(exp->node, env);
        case EXPRESSION_IDENTIFIER:
          return eval_identifier(((Identifier *)exp->node), env);
        case EXPRESSION_IF:
          return eval_if_expression(((IfExpression *)exp->node), env);
        case EXPRESSION_BOOLEAN_LITERAL:
          return eval(exp->node, BOOLEAN_LITERAL_NODE, env);
        case EXPRESSION_INTEGER_LITERAL:
          return eval(exp->node, INTEGER_LITERAL_NODE, env);
      }
    }
  }
  return object;
}

Object eval_statement(Statement *stmt, Env *env) {
  return eval(stmt->node, ast_statement_node_type(stmt), env);
}

Object eval_program(List *statements, Env *env) {
  Object object;
  List *current = statements;
  for (; current != NULL; current = current->next)
    if (current->item != NULL) {
      Statement *stmt = (Statement *)current->item;
      object = eval_statement(stmt, env);
      if (object.type == RETURN_VALUE_OBJ) {
        return *object.value.return_value;
      } else if (object.type == ERROR_OBJ) {
        return object;
      }
    }
  return object;
}

Object eval_block_statement(List *statements, Env *env) {
  Object object;
  List *current = statements;
  for (; current != NULL; current = current->next)
    if (current->item != NULL) {
      Statement *stmt = (Statement *)current->item;
      object = eval_statement(stmt, env);
      if (object.type == RETURN_VALUE_OBJ || object.type == ERROR_OBJ) {
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
    return error("unknown operator: -%s", (char *[1]){object_type(right)}, 1);
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
      return error(
        "unknown operator: %s%s", (char *[2]){operator, object_type(right)}, 2);
  }
}

Object eval_infix_expression(char *operator, Object left, Object right) {
  if (left.type == INTEGER_OBJ && right.type == INTEGER_OBJ) {
    return eval_integer_infix_expression(operator, left, right);
  }

  if (left.type == STRING_OBJ && right.type == STRING_OBJ) {
    return eval_string_infix_expression(operator, left, right);
  }

  switch (*operator) {
    case '=':  // `==`
      return left.value.b == right.value.b ? TRUE : FALSE;
    case '!':  // `!=`
      return left.value.b != right.value.b ? TRUE : FALSE;
  }

  if (left.type != right.type) {
    return error("type mismatch: %s %s %s",
      (char *[3]){object_type(left), operator, object_type(right)}, 3);
  }

  return error("unknown operator: %s %s %s",
    (char *[3]){object_type(left), operator, object_type(right)}, 3);
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
  }
  return error("unknown operator: %s %s %s",
    (char *[3]){object_type(left), operator, object_type(right)}, 3);
}

Object eval_string_infix_expression(char *operator, Object left, Object right) {
  if (*operator!= '+')
    return error("unknown operator: %s %s %s",
      (char *[3]){object_type(left), operator, object_type(right)}, 3);
  char *left_val = left.value.str;
  char *right_val = right.value.str;
  char *combined = malloc(strlen(left_val) + strlen(right_val) + 1);
  sprintf(combined, "%s%s", left_val, right_val);
  return (Object){STRING_OBJ, {.str = combined}};
}

bool is_truthy(Object obj) {
  if (obj.type == NULL_OBJ)
    return false;
  else if (obj.type == BOOLEAN_OBJ && obj.value.b == false)
    return false;
  else
    return true;
}

Object eval_if_expression(IfExpression *if_exp, Env *env) {
  Object condition = eval(if_exp->condition, EXPRESSION_NODE, env);
  if (is_error(condition))
    return condition;
  if (is_truthy(condition))
    return eval(if_exp->consequence, BLOCK_STATEMENTS_NODE, env);
  else if (if_exp->alternative != NULL)
    return eval(if_exp->alternative, BLOCK_STATEMENTS_NODE, env);
  else
    return M_NULL;
}

Object error(char *fmt, char **types, int num_types) {
  char *err_msg = malloc(1024);
  switch (num_types) {
    case 1:
      sprintf(err_msg, fmt, types[0]);
      break;
    case 2:
      sprintf(err_msg, fmt, types[0], types[1]);
      break;
    case 3:
      sprintf(err_msg, fmt, types[0], types[1], types[2]);
      break;
    default:
      printf("ERROR: unhandled error types amount\n");
      exit(EXIT_FAILURE);
  }
  Object error = {ERROR_OBJ, {.str = err_msg}};
  return error;
}

bool is_error(Object object) {
  return object.type == ERROR_OBJ;
}

Object eval_identifier(Identifier *ident, Env *env) {
  if (env_has(env, ident->value)) {
    return env_get(env, ident->value);
  }

  Object built_in = get_builtin(ident->value);
  if (built_in.type == BUILT_IN_OBJ) {
    return built_in;
  }

  return error("identifier not found: %s", (char *[1]){ident->value}, 1);
}

List *eval_expressions(List *expressions, Env *env) {
  List *objects = NULL;
  List *current = expressions;
  for (; current != NULL; current = current->next) {
    if (current->item != NULL) {
      Object evaluated = eval(current->item, EXPRESSION_NODE, env);
      if (is_error(evaluated)) {
        objects->item = object_copy(evaluated);
        objects->next = NULL;
        return objects;
      }
      objects = list_append(objects, object_copy(evaluated));
    }
  }
  return objects;
}

Object apply_function(Object fn_obj, List *args) {
  if (fn_obj.type == FUNCTION_OBJ) {
    Function *fn = fn_obj.value.fn;
    Env *extended_env = extend_function_env(fn, args);
    Object evaluated = eval(fn->body, BLOCK_STATEMENTS_NODE, extended_env);
    return unwrap_return_value(evaluated);
  }

  if (fn_obj.type == BUILT_IN_OBJ) {
    return (fn_obj.value.builtin_fn)(args);
  }

  return error("not a function: %s", (char *[1]){object_type(fn_obj)}, 1);
}

Object unwrap_return_value(Object obj) {
  if (obj.type == RETURN_VALUE_OBJ)
    return *obj.value.return_value;
  return obj;
}

Env *extend_function_env(Function *fn, List *args) {
  Env *env = env_new_enclosed(fn->env);

  if (list_count(args) != list_count(fn->parameters)) {
    printf("Error: num params does not match num args\n");
    exit(EXIT_FAILURE);
  }

  List *current_arg = args;
  List *current_param = fn->parameters;

  for (; current_arg != NULL && current_param != NULL;
       current_arg = current_arg->next, current_param = current_param->next) {
    if (current_arg->item != NULL && current_param->item != NULL) {
      Identifier *param = current_param->item;
      Object *arg = current_arg->item;
      env_set(env, param->value, *arg);
    }
  }

  return env;
}

Object eval_index_expression(Object left, Object index) {
  if (left.type == ARRAY_OBJ && index.type == INTEGER_OBJ)
    return eval_array_index_expression(left, index);
  if (left.type == HASH_OBJ)
    return eval_hash_index_expression(left, index);
  return error(
    "index operator not supported: %s", (char *[1]){object_type(left)}, 1);
}

Object eval_array_index_expression(Object array, Object index) {
  List *elements = array.value.list;
  int idx = index.value.i;
  int max = list_count(elements) - 1;

  if (idx < 0 || idx > max)
    return M_NULL;

  List *current = elements;
  int i = 0;
  while (true) {
    if (i == idx) {
      Object *obj = current->item;
      return *obj;
    }
    current = current->next;
    i += 1;
  }
}

Object eval_hash_index_expression(Object hash, Object index) {
  char *index_hash = object_hash(index);
  if (index_hash == NULL)
    return error(
      "unusable as hash key: %s", (char *[1]){object_type(index)}, 1);

  List *current = hash.value.list;
  for (; current != NULL; current = current->next) {
    HashPair *pair = current->item;
    char *key_hash = object_hash(*pair->key);
    if (strcmp(key_hash, index_hash) == 0)
      return *pair->value;
  }
  return M_NULL;
}

Object eval_hash_literal(HashLiteralExpression *hash_lit_exp, Env *env) {
  List *exp_pairs = hash_lit_exp->pairs;  // List<HashLiteralPair>
  List *obj_pairs = NULL;                 // List<Object>

  int num_pairs = list_count(exp_pairs);
  List *current = exp_pairs;

  for (int i = 0; i < num_pairs; i++) {
    HashLiteralPair *current_exp_pair = current->item;
    Object key = eval(current_exp_pair->key, EXPRESSION_NODE, env);
    if (is_error(key))
      return key;

    if (object_hash(key) == NULL)
      return error(
        "unusable as hash key: %s", (char *[1]){object_type(key)}, 1);

    Object value = eval(current_exp_pair->value, EXPRESSION_NODE, env);
    if (is_error(value))
      return value;

    HashPair *obj_pair = malloc(sizeof(HashPair));
    obj_pair->key = object_copy(key);
    obj_pair->value = object_copy(value);
    obj_pairs = list_append(obj_pairs, obj_pair);
    current = current->next;
  }

  Object hash_obj = {.type = HASH_OBJ, .value = {.list = obj_pairs}};
  return hash_obj;
}
