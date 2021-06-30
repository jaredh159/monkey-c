#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../utils/list.h"
#include "object.h"

Object wrong_num_args_error(int got, int want);
Object wrong_arg_type_error(char *fn, char *expected_type, Object arg);

Object builtin_puts(List *args) {
  for (List *cur = args; cur != NULL; cur = cur->next) {
    Object *object = cur->item;
    puts(object_inspect(*object));
  }
  return M_NULL;
}

Object builtin_len(List *args) {
  if (list_count(args) != 1) {
    return wrong_num_args_error(list_count(args), 1);
  }

  Object arg = *((Object *)args->item);
  switch (arg.type) {
    case STRING_OBJ:
      return (Object){INTEGER_OBJ, {.i = strlen(arg.value.str)}};
    case ARRAY_OBJ:
      return (Object){INTEGER_OBJ, {.i = list_count(arg.value.list)}};
    default: {
      char *msg = malloc(100);
      sprintf(msg, "argument to `len` not supported, got %s", object_type(arg));
      Object err = {ERROR_OBJ, {.str = msg}};
      return err;
    }
  }
}

Object builtin_first(List *args) {
  if (list_count(args) != 1) {
    return wrong_num_args_error(list_count(args), 1);
  }

  Object arg = *((Object *)args->item);
  if (arg.type != ARRAY_OBJ) {
    return wrong_arg_type_error("first", "ARRAY", arg);
  }

  if (list_count(arg.value.list) > 0) {
    Object *pi = arg.value.list->item;
    return *pi;
  }

  return M_NULL;
}

Object builtin_last(List *args) {
  if (list_count(args) != 1) {
    return wrong_num_args_error(list_count(args), 1);
  }

  Object arg = *((Object *)args->item);
  if (arg.type != ARRAY_OBJ) {
    return wrong_arg_type_error("first", "ARRAY", arg);
  }

  if (list_count(arg.value.list) == 0) {
    return M_NULL;
  }

  List *current = arg.value.list;
  while (current != NULL && current->next != NULL) current = current->next;

  Object *last = current->item;
  return *last;
}

Object builtin_rest(List *args) {
  if (list_count(args) != 1) {
    return wrong_num_args_error(list_count(args), 1);
  }

  Object arg = *((Object *)args->item);
  if (arg.type != ARRAY_OBJ) {
    return wrong_arg_type_error("first", "ARRAY", arg);
  }

  if (list_count(arg.value.list) == 0) {
    return M_NULL;
  }

  List *current = arg.value.list->next;
  List *rest = NULL;
  while (current != NULL) {
    Object *obj = current->item;
    rest = list_append(rest, object_copy(*obj));
    current = current->next;
  }

  Object new_array;
  new_array.type = ARRAY_OBJ;
  new_array.value.list = rest;
  return new_array;
}

Object builtin_push(List *args) {
  if (list_count(args) != 2) {
    return wrong_num_args_error(list_count(args), 2);
  }

  Object arr_obj = *((Object *)args->item);
  if (arr_obj.type != ARRAY_OBJ) {
    return wrong_arg_type_error("first", "ARRAY", arr_obj);
  }

  // copy the array
  List *current = arr_obj.value.list;
  List *new_list = NULL;
  while (current != NULL) {
    Object *obj = current->item;
    new_list = list_append(new_list, object_copy(*obj));
    current = current->next;
  }

  // now push the new item
  Object new_element = *((Object *)args->next->item);
  new_list = list_append(new_list, object_copy(new_element));

  Object new_arr_obj;
  new_arr_obj.type = ARRAY_OBJ;
  new_arr_obj.value.list = new_list;
  return new_arr_obj;
}

Object get_builtin(char *name) {
  if (strcmp("len", name) == 0) {
    return (Object){BUILT_IN_OBJ, {.builtin_fn = builtin_len}};
  } else if (strcmp("first", name) == 0) {
    return (Object){BUILT_IN_OBJ, {.builtin_fn = builtin_first}};
  } else if (strcmp("rest", name) == 0) {
    return (Object){BUILT_IN_OBJ, {.builtin_fn = builtin_rest}};
  } else if (strcmp("push", name) == 0) {
    return (Object){BUILT_IN_OBJ, {.builtin_fn = builtin_push}};
  } else if (strcmp("puts", name) == 0) {
    return (Object){BUILT_IN_OBJ, {.builtin_fn = builtin_puts}};
  } else if (strcmp("last", name) == 0) {
    return (Object){BUILT_IN_OBJ, {.builtin_fn = builtin_last}};
  }
  return (Object){NOT_FOUND_OBJ, {.i = 0}};
}

Object wrong_num_args_error(int got, int want) {
  char *msg = malloc(100);
  sprintf(msg, "wrong number of arguments. got=%d, want=%d", got, want);
  Object err = {ERROR_OBJ, {.str = msg}};
  return err;
}

Object wrong_arg_type_error(char *fn, char *expected_type, Object arg) {
  char *msg = malloc(100);
  sprintf(msg, "argument to `%s` must be %s, got %s", fn, expected_type,
    object_type(arg));
  Object err = {ERROR_OBJ, {.str = msg}};
  return err;
}
