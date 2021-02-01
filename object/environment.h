#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__

#include <stdbool.h>
#include "../utils/list.h"
#include "object.h"

typedef struct Env {
  List *store;
  struct Env *outer;
} Env;

typedef struct Binding {
  char *name;
  Object value;
} Binding;

Env *env_new(void);
bool env_has(Env *env, char *name);
Object env_get(Env *env, char *name);
void env_set(Env *env, char *name, Object val);

#endif  // __ENVIRONMENT_H__
