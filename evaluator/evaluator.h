#ifndef __EVALUATOR_H__
#define __EVALUATOR_H__

#include "../object/object.h"

Object eval(void *node, NodeType type, Env *env);
Object get_builtin(char *name);

#endif  // __EVALUATOR_H__
