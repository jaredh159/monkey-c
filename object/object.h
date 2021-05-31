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
  ARRAY_OBJ,
  HASH_OBJ,
  BUILT_IN_OBJ,
  NOT_FOUND_OBJ,
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
    struct Object (*builtin_fn)(List *args);
    List *list;  // List<Object> (for array elements) | List<HashPair>
  } value;
} Object;

extern Object M_NULL;
extern Object TRUE;
extern Object FALSE;

typedef struct HashPair {
  Object *key;
  Object *value;
} HashPair;

typedef struct Binding {
  char *name;
  Object value;
} Binding;

char *object_inspect(Object object);
char *object_type(Object object);
void object_print(Object object);
Object *object_copy(const Object proto);
char *object_hash(const Object object);
bool is_truthy(Object obj);

Env *env_new(void);
Env *env_new_enclosed(Env *outer);
bool env_has(Env *env, char *name);
Object env_get(Env *env, char *name);
void env_set(Env *env, char *name, Object val);

#endif  // __OBJECT_H__
