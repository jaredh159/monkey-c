#include "object.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INSPECT_STR_LEN 1024
char *function_inspect(Function *fn);
char *array_inspect(List *elements);

char *object_inspect(Object object) {
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
      return array_inspect(object.value.array_elements);
    case FUNCTION_OBJ:
      return function_inspect(object.value.fn);
    case NULL_OBJ:
      return "null";
    case BUILT_IN_OBJ:
      return "builtin function";
    case ERROR_OBJ:
      sprintf(inspect_str, "ERROR: %s", object.value.str);
      break;
  }
  return inspect_str;
}

char *object_type(Object object) {
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
    case ERROR_OBJ:
      return "ERROR";
  }
  printf("ERROR: unhandled type in object_type()\n");
  exit(1);
}

void object_print(Object object) {
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
      copy->value.array_elements = proto.value.array_elements;
      break;
    case NULL_OBJ:
      break;
    case ERROR_OBJ:
    case STRING_OBJ: /* intentional fallthrough */
      copy->value.str = strdup(proto.value.str);
      break;
    default:
      printf("ERROR: unhandled object copy type %s\n", object_type(proto));
      exit(1);
  }
  return copy;
}
