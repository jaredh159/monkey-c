#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../utils/list.h"
#include "object.h"

Env *env_new(void) {
  Env *env = malloc(sizeof(Env));
  env->store = NULL;
  env->outer = NULL;
  return env;
}

Env *env_new_enclosed(Env *outer) {
  Env *env = env_new();
  env->outer = outer;
  return env;
}

bool env_has(Env *env, char *name) {
  return env_get(env, name).type != ENV_LOOKUP_NOT_FOUND_OBJ;
}

Object env_get(Env *env, char *name) {
  List *current = env->store;
  for (; current != NULL; current = current->next) {
    if (current->item == NULL)
      continue;
    Binding *binding = current->item;
    if (strcmp(binding->name, name) == 0)
      return binding->value;
  }

  if (env->outer != NULL)
    return env_get(env->outer, name);

  return (Object){ENV_LOOKUP_NOT_FOUND_OBJ, {.i = 0}};
}

void env_set(Env *env, char *name, Object val) {
  Binding *binding = malloc(sizeof(Binding));
  binding->name = name;
  binding->value = val;
  List *store = malloc(sizeof(List));
  store->item = binding;
  store->next = env->store;
  env->store = store;
}
