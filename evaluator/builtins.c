#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../object/object.h"
#include "../utils/list.h"

Object builtin_len(List *args) {
  if (list_count(args) != 1) {
    char *msg = malloc(100);
    sprintf(msg, "wrong number of arguments. got=%d, want=1", list_count(args));
    Object err = {ERROR_OBJ, {.str = msg}};
    return err;
  }

  Object arg = *((Object *)args->item);
  switch (arg.type) {
    case STRING_OBJ: {
      return (Object){INTEGER_OBJ, {.i = strlen(arg.value.str)}};
    }
    default: {
      char *msg = malloc(100);
      sprintf(msg, "argument to `len` not supported, got %s", object_type(arg));
      Object err = {ERROR_OBJ, {.str = msg}};
      return err;
    }
  }
}

Object get_builtin(char *name) {
  if (strcmp("len", name) == 0) {
    return (Object){BUILT_IN_OBJ, {.builtin_fn = builtin_len}};
  }
  return (Object){NOT_FOUND_OBJ, {.i = 0}};
}
