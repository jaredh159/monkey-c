#ifndef __EVALUATOR_H__
#define __EVALUATOR_H__

#include "../object/object.h"

enum {
  PROGRAM_NODE,
  EXPRESSION_NODE,
  EXPRESSION_STATEMENT_NODE,
  INTEGER_LITERAL_NODE,
  BOOLEAN_LITERAL_NODE,
  BLOCK_STATEMENTS_NODE,
};

typedef int NodeType;

Object eval(void *node, NodeType type);

#endif  // __EVALUATOR_H__
