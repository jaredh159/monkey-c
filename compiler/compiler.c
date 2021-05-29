#include "compiler.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../parser/parser.h"

#define BACKPATCH_LATER i(255)

typedef struct EmittedInstruction {
  OpCode op_code;
  int position;
} EmittedInstruction;

static Instruct* instructions = NULL;
static ConstantPool* constant_pool = NULL;
static EmittedInstruction last_instruction = {0};
static EmittedInstruction previous_instruction = {0};
static IntBag _ = {0};

static CompilerErr compile_statements(List* statements);
static int add_constant(Object* object);
static int emit(OpCode op_code, IntBag operands);
static int add_instruction(Instruct* instructions);
static void set_last_instruction(OpCode op_code, int position);
static void remove_last_pop(void);
static void replace_instruction(int pos, Instruct* new_instruction);
static void change_operand(int op_code_pos, int operand);

void compiler_init(void) {
  if (instructions != NULL) {
    free(constant_pool->constants);
    free(instructions->bytes);
    free(instructions);
    free(constant_pool);
  }
  instructions = malloc(sizeof(Instruct));
  constant_pool = malloc(sizeof(ConstantPool));
  constant_pool->length = 0;
  constant_pool->constants = malloc(sizeof(Object) * MAX_CONSTANTS);
  instructions->length = 0;
  instructions->bytes = malloc(sizeof(Byte) * MAX_INSTRUCTIONS);
}

CompilerErr compile(void* node, NodeType type) {
  CompilerErr err = malloc(100);
  switch (type) {
    case PROGRAM_NODE:           // fallthrough
    case BLOCK_STATEMENTS_NODE:  // fallthrough
      err = compile_statements(((BlockStatement*)node)->statements);
      if (err)
        return err;
      break;
    case EXPRESSION_STATEMENT_NODE:
      err = compile(((ExpressionStatement*)node)->expression, EXPRESSION_NODE);
      if (err)
        return err;
      emit(OP_POP, _);
      break;
    case INTEGER_LITERAL_NODE: {
      Object* int_lit = malloc(sizeof(Object));
      int_lit->type = INTEGER_OBJ;
      int_lit->value.i = ((IntegerLiteral*)node)->value;
      int constant_idx = add_constant(int_lit);
      emit(OP_CONSTANT, i(constant_idx));
    } break;
    case BOOLEAN_LITERAL_NODE:
      if (((BooleanLiteral*)node)->value) {
        emit(OP_TRUE, _);
      } else {
        emit(OP_FALSE, _);
      }
      break;
    case EXPRESSION_NODE: {
      Expression* exp = node;
      switch (exp->type) {
        case EXPRESSION_INFIX: {
          InfixExpression* infix = exp->node;
          if (infix->operator[0] == '<') {
            err = compile(infix->right, EXPRESSION_NODE);
            if (err)
              return err;
            err = compile(infix->left, EXPRESSION_NODE);
            if (err)
              return err;
            emit(OP_GREATER_THAN, _);
            return NULL;
          }
          err = compile(infix->left, EXPRESSION_NODE);
          if (err)
            return err;
          err = compile(infix->right, EXPRESSION_NODE);
          if (err)
            return err;
          switch ((int)infix->operator[0]) {
            case '+':
              emit(OP_ADD, _);
              break;
            case '-':
              emit(OP_SUB, _);
              break;
            case '/':
              emit(OP_DIV, _);
              break;
            case '>':
              emit(OP_GREATER_THAN, _);
              break;
            case '*':
              emit(OP_MUL, _);
              break;
            case '=':
              emit(OP_EQUAL, _);
              break;
            case '!':
              emit(OP_NOT_EQUAL, _);
              break;
            default:
              sprintf(err, "unknown operator %s", infix->operator);
              return err;
          }
        } break;
        case EXPRESSION_PREFIX: {
          PrefixExpression* prefix = exp->node;
          err = compile(prefix->right, EXPRESSION_NODE);
          if (err)
            return err;
          switch ((int)prefix->operator[0]) {
            case '!':
              emit(OP_BANG, _);
              break;
            case '-':
              emit(OP_MINUS, _);
              break;
            default:
              sprintf(err, "unknown operator %s", prefix->operator);
              return err;
          }
        } break;
        case EXPRESSION_BOOLEAN_LITERAL:
          err = compile(exp->node, BOOLEAN_LITERAL_NODE);
          if (err)
            return err;
          break;
        case EXPRESSION_INTEGER_LITERAL:
          err = compile(exp->node, INTEGER_LITERAL_NODE);
          if (err)
            return err;
          break;
        case EXPRESSION_IF: {
          IfExpression* if_exp = (IfExpression*)exp->node;
          err = compile(if_exp->condition, EXPRESSION_NODE);
          if (err)
            return err;

          int jump_not_truthy_pos = emit(OP_JUMP_NOT_TRUTHY, BACKPATCH_LATER);
          err = compile(if_exp->consequence, BLOCK_STATEMENTS_NODE);
          if (err)
            return err;

          if (last_instruction.op_code == OP_POP) {
            remove_last_pop();
          }

          if (if_exp->alternative == NULL) {
            int after_conseq_pos = instructions->length;
            change_operand(jump_not_truthy_pos, after_conseq_pos);
          } else {
            int jump_pos = emit(OP_JUMP, BACKPATCH_LATER);

            int after_conseq_pos = instructions->length;
            change_operand(jump_not_truthy_pos, after_conseq_pos);

            err = compile(if_exp->alternative, BLOCK_STATEMENTS_NODE);
            if (err)
              return err;

            if (last_instruction.op_code == OP_POP) {
              remove_last_pop();
            }

            int after_alt_pos = instructions->length;
            change_operand(jump_pos, after_alt_pos);
          }
        } break;
      }
      break;
    }
  }
  return NULL;
}

int emit(OpCode op, IntBag operands) {
  Instruct* instruction = code_make_nv(op, operands);
  int pos = add_instruction(instruction);
  set_last_instruction(op, pos);
  return pos;
}

static void replace_instruction(int pos, Instruct* new_instruction) {
  for (int i = 0; i < new_instruction->length; i++) {
    instructions->bytes[pos + i] = new_instruction->bytes[i];
  }
}

static void change_operand(int op_code_pos, int operand) {
  OpCode op = instructions->bytes[op_code_pos];
  Instruct* new_instruction = code_make(op, operand);
  replace_instruction(op_code_pos, new_instruction);
}

static void remove_last_pop(void) {
  instructions->length--;
  last_instruction = previous_instruction;
}

void set_last_instruction(OpCode op_code, int position) {
  previous_instruction = last_instruction;
  last_instruction.op_code = op_code;
  last_instruction.position = position;
}

CompilerErr compile_statements(List* statements) {
  CompilerErr err = NULL;
  List* current = statements;
  for (; current != NULL; current = current->next) {
    if (current->item != NULL) {
      Statement* stmt = (Statement*)current->item;
      err = compile(stmt->node, ast_statement_node_type(stmt));
      if (err)
        return err;
    }
  }
  return NULL;
}

Bytecode* compiler_bytecode(void) {
  Bytecode* bytecode = malloc(sizeof(Bytecode));
  bytecode->constants = constant_pool;
  bytecode->instructions = instructions;
  return bytecode;
}

ConstantPool* make_constant_pool(int len, ...) {
  ConstantPool* pool = malloc(sizeof(ConstantPool));
  pool->length = len;

  if (len == 0) {
    pool->constants = NULL;
    return pool;
  }

  va_list ap;
  va_start(ap, len);

  Object* constants = malloc(sizeof(Object) * len);
  for (int i = 0; i < len; i++) {
    Object constant = va_arg(ap, Object);
    constants[i] = constant;
  }

  va_end(ap);

  pool->constants = constants;
  return pool;
}

int add_constant(Object* obj) {
  constant_pool->constants[constant_pool->length] = *obj;
  constant_pool->length += 1;
  return constant_pool->length - 1;
}

int add_instruction(Instruct* instruction) {
  int insert_idx = instructions->length;
  instructions->length += instruction->length;
  for (int i = 0; i < instruction->length; i++) {
    instructions->bytes[insert_idx + i] = instruction->bytes[i];
  }
  free(instruction);
  return insert_idx;
}
