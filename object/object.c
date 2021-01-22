#include "object.h"
#include <stdio.h>

char inspect_str[1024];

char *object_inspect(Object object) {
  switch (object.type) {
    case INTEGER_OBJ:
      return sprintf(inspect_str, "%d", object.value.i);
    case BOOLEAN_OBJ:
      return sprintf(inspect_str, "%s", object.value.b ? "true" : "false");
    case NULL_OBJ:
      return "null";
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
  }
  return "UNKNOWN";
}
