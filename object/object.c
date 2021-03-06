#include "object.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INSPECT_STR_LEN 1024
char *function_inspect(Function *fn);
char *array_inspect(List *elements);
char *hash_inspect(List *pairs);

Object M_NULL = {NULL_OBJ, {0}};
Object TRUE = {BOOLEAN_OBJ, {.b = true}};
Object FALSE = {BOOLEAN_OBJ, {.b = false}};

char *object_inspect(const Object object) {
  char *inspect_str = malloc(INSPECT_STR_LEN);
  switch (object.type) {
    case INTEGER_OBJ:
      sprintf(inspect_str, "%d", object.value.i);
      break;
    case BOOLEAN_OBJ:
      sprintf(inspect_str, "%s", object.value.b ? "true" : "false");
      break;
    case STRING_OBJ:
      return object.value.str;
    case RETURN_VALUE_OBJ:
      sprintf(inspect_str, "wrapped return_value { %s }",
        object_inspect(*object.value.return_value));
      break;
    case ARRAY_OBJ:
      return array_inspect(object.value.list);
    case FUNCTION_OBJ:
      return function_inspect(object.value.fn);
    case HASH_OBJ:
      return hash_inspect(object.value.list);
    case NULL_OBJ:
      return "null";
    case BUILT_IN_OBJ:
      return "builtin function";
    case ERROR_OBJ:
      sprintf(inspect_str, "ERROR: %s", object.value.str);
      break;
    case CLOSURE_OBJ:
      return "Closure";
    case COMPILED_FUNCTION_OBJ:
      return "CompiledFunction";
    default:
      printf(
        "unhandled object type %s for object_inspect())", object_type(object));
      exit(EXIT_FAILURE);
  }
  return inspect_str;
}

char *object_type(const Object object) {
  switch (object.type) {
    case INTEGER_OBJ:
      return "INTEGER";
    case BOOLEAN_OBJ:
      return "BOOLEAN";
    case NULL_OBJ:
      return "NULL";
    case RETURN_VALUE_OBJ:
      return "RETURN_VALUE";
    case FUNCTION_OBJ:
      return "FUNCTION";
    case STRING_OBJ:
      return "STRING";
    case ARRAY_OBJ:
      return "ARRAY";
    case BUILT_IN_OBJ:
      return "BUILT_IN";
    case HASH_OBJ:
      return "HASH";
    case COMPILED_FUNCTION_OBJ:
      return "COMPILED_FUNCTION_OBJ";
    case CLOSURE_OBJ:
      return "CLOSURE_OBJ";
    case ERROR_OBJ:
      return "ERROR";
  }
  printf("ERROR: unhandled type in object_type() = %d\n", object.type);
  exit(EXIT_FAILURE);
}

void object_print(const Object object) {
  printf("object.type=%s, object.value=%s\n", object_type(object),
    object_inspect(object));
}

char *function_inspect(Function *fn) {
  char *fn_inspect_str = malloc(INSPECT_STR_LEN);
  fn_inspect_str[0] = '\0';
  strcpy(fn_inspect_str, "fn(");
  int num_params = list_count(fn->parameters);
  List *current = fn->parameters;
  for (int i = 0; i < num_params; i++) {
    Identifier *ident = current->item;
    strcat(fn_inspect_str, ident->value);
    if (i < num_params - 1)
      strcat(fn_inspect_str, ", ");
    current = current->next;
  }
  strcat(fn_inspect_str, ") {\n");
  strcat(fn_inspect_str, block_statement_string(fn->body));
  strcat(fn_inspect_str, "\n}");
  return fn_inspect_str;
}

char *array_inspect(List *elements) {
  char *array_inspect_str = malloc(INSPECT_STR_LEN);
  array_inspect_str[0] = '[';
  array_inspect_str[1] = '\0';
  int num_elements = list_count(elements);
  List *current = elements;
  for (int i = 0; i < num_elements; i++) {
    Object *obj = current->item;
    strcat(array_inspect_str, object_inspect(*obj));
    if (i < num_elements - 1)
      strcat(array_inspect_str, ", ");
    current = current->next;
  }
  strcat(array_inspect_str, "]");
  return array_inspect_str;
}

char *hash_inspect(List *pairs) {
  char *hash_str = malloc(INSPECT_STR_LEN);
  strcat(hash_str, "{");
  int num_elements = list_count(pairs);
  List *current = pairs;
  for (int i = 0; i < num_elements; i++) {
    HashPair *pair = current->item;
    strcat(hash_str, object_inspect(*pair->key));
    strcat(hash_str, ": ");
    strcat(hash_str, object_inspect(*pair->value));
    if (i < num_elements - 1)
      strcat(hash_str, ", ");
    current = current->next;
  }
  strcat(hash_str, "}");
  return hash_str;
}

Object *object_copy(const Object proto) {
  Object *copy = malloc(sizeof(Object));
  if (copy == NULL)
    return NULL;
  copy->type = proto.type;
  switch (proto.type) {
    case INTEGER_OBJ:
      copy->value.i = proto.value.i;
      break;
    case BOOLEAN_OBJ:
      copy->value.b = proto.value.b;
      break;
    case RETURN_VALUE_OBJ:
      copy->value.return_value = object_copy(*proto.value.return_value);
      break;
    case FUNCTION_OBJ:
      copy->value.fn = proto.value.fn;
      break;
    case ARRAY_OBJ:
    case HASH_OBJ: /* intentional fallthrough */
      copy->value.list = proto.value.list;
      break;
    case NULL_OBJ:
      break;
    case ERROR_OBJ:
    case STRING_OBJ: /* intentional fallthrough */
      copy->value.str = strdup(proto.value.str);
      break;
    default:
      printf("ERROR: unhandled object copy type %s\n", object_type(proto));
      exit(EXIT_FAILURE);
  }
  return copy;
}

char *object_hash(const Object object) {
  char *hash;
  switch (object.type) {
    case STRING_OBJ:
      hash = malloc(strlen(object.value.str) + 2);
      sprintf(hash, "S=%s", object.value.str);
      return hash;
    case BOOLEAN_OBJ:
      hash = malloc(8);
      sprintf(hash, "B=%s", object.value.b ? "true" : "false");
      return hash;
    case INTEGER_OBJ:
      hash = malloc(50);
      sprintf(hash, "I=%d", object.value.i);
      return hash;
    default:
      return NULL;
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
