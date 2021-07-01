#ifndef __EVALUATOR_H__
#define __EVALUATOR_H__

#include "../object/object.h"

Object eval(void *node, NodeType type, Env *env);

#endif  // __EVALUATOR_H__
