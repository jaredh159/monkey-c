#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <stdbool.h>
#include "../ast/ast.h"
#include "../utils/list.h"

enum ObjectTypes {
  FUNCTION_OBJ,
  INTEGER_OBJ,
  BOOLEAN_OBJ,
  STRING_OBJ,
  NULL_OBJ,
  RETURN_VALUE_OBJ,
  ERROR_OBJ,
  ENV_LOOKUP_NOT_FOUND_OBJ,
};

typedef int ObjectType;

typedef struct Env {
  List *store;
  struct Env *outer;
} Env;

typedef struct Function {
  List *parameters;
  BlockStatement *body;
  Env *env;
} Function;

typedef struct Object {
  ObjectType type;
  union {
    int i;
    bool b;
    struct Object *return_value;
    char *str;
    Function *fn;
  } value;
} Object;

typedef struct Binding {
  char *name;
  Object value;
} Binding;

char *object_inspect(Object object);
char *object_type(Object object);
void object_print(Object object);
Object *object_copy(const Object proto);

Env *env_new(void);
Env *env_new_enclosed(Env *outer);
bool env_has(Env *env, char *name);
Object env_get(Env *env, char *name);
void env_set(Env *env, char *name, Object val);

#endif  // __OBJECT_H__
