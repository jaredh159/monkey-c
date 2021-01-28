#include "object.h"
#include <stdio.h>

char inspect_str[1024];

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
