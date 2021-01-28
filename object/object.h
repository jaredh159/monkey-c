#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <stdbool.h>

enum {
  INTEGER_OBJ,
  BOOLEAN_OBJ,
  NULL_OBJ,
  RETURN_VALUE_OBJ,
};

typedef int ObjectType;

typedef struct Object {
  ObjectType type;
  union {
    int i;
    bool b;
    struct Object *return_value;
  } value;
} Object;

char *object_inspect(Object object);
char *object_type(Object object);
void object_print(Object object);

#endif  // __OBJECT_H__
