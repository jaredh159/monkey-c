#include "object.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char inspect_str[1024];

char *function_inspect(Function *fn);

char *object_inspect(Object object) {
  switch (object.type) {
    case INTEGER_OBJ:
      sprintf(inspect_str, "%d", object.value.i);
      break;
    case BOOLEAN_OBJ:
      sprintf(inspect_str, "%s", object.value.b ? "true" : "false");
      break;
    case RETURN_VALUE_OBJ:
      return object_inspect(*object.value.return_value);
    case FUNCTION_OBJ:
      return function_inspect(object.value.fn);
    case NULL_OBJ:
      return "null";
    case ERROR_OBJ:
      sprintf(inspect_str, "ERROR: %s", object.value.message);
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
    case ERROR_OBJ:
      return "ERROR";
  }
  printf("num: %d\n", object.type);
  return "UNKNOWN";
}

void object_print(Object object) {
  printf("object.type=%s, object.value=%s\n", object_type(object),
    object_inspect(object));
}

char *function_inspect(Function *fn) {
  inspect_str[0] = '\0';
  strcpy(inspect_str, "fn(");
  int num_params = list_count(fn->parameters);
  List *current = fn->parameters;
  for (int i = 0; i < num_params; i++) {
    Identifier *ident = current->item;
    strcat(inspect_str, ident->value);
    if (i < num_params - 1)
      strcat(inspect_str, ", ");
  }
  strcat(inspect_str, ") {\n");
  strcat(inspect_str, block_statement_string(fn->body));
  strcat(inspect_str, "\n}");
  return inspect_str;
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
      copy->value.return_value = proto.value.return_value;
      break;
    case FUNCTION_OBJ:
      copy->value.fn = proto.value.fn;
      break;
    case NULL_OBJ:
      break;
    case ERROR_OBJ:
      copy->value.message = strdup(proto.value.message);
      break;
  }
  return copy;
}
