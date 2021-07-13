#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../code/code.h"
#include "../compiler/compiler.h"
#include "../compiler/symbol_table.h"
#include "../evaluator/evaluator.h"
#include "../lexer/lexer.h"
#include "../object/object.h"
#include "../parser/parser.h"
#include "../token/token.h"
#include "../utils/argv.h"
#include "../utils/colors.h"
#include "../vm/vm.h"

typedef struct {
  Object object;
  double duration;
} ExecResult;

static ExecResult exec(char* input, bool compile);
static ExecResult exec_compile(Program* program);
static ExecResult exec_interpret(Program* program);
static char* input_from_file(int argc, char** argv);

void run(int argc, char** argv) {
  bool measure = argv_has_flag('m', argc, argv);
  bool compile = argv_has_flag('i', argc, argv);

  char* input = "";
  int eval_flag_index = argv_idx("-e", argc, argv);
  if (eval_flag_index != -1) {
    input = argv[eval_flag_index + 1];
  } else {
    input = input_from_file(argc, argv);
  }

  ExecResult result = exec(input, compile);
  printf("%s\n", object_inspect(result.object));
  if (measure) {
    printf("execution time: %f\n", result.duration);
  }
}

static ExecResult exec(char* input, bool compile) {
  Program* program = parse_program(input);
  if (parser_num_errors() > 0) {
    parser_print_errors();
    exit(EXIT_FAILURE);
  }

  if (compile) {
    return exec_compile(program);
  } else {
    return exec_interpret(program);
  }
}

static ExecResult exec_compile(Program* program) {
  clock_t start, end;
  Compiler compiler = compiler_new();
  char* compiler_err = compile(compiler, program, PROGRAM_NODE);
  if (compiler_err) {
    printf("compiler error: %s\n", compiler_err);
    exit(EXIT_FAILURE);
  }

  Vm vm = vm_new(compiler_bytecode(compiler));
  start = clock();
  char* vm_err = vm_run(vm);
  end = clock();
  if (vm_err) {
    printf("vm error: %s\n", vm_err);
    exit(EXIT_FAILURE);
  }

  ExecResult result;
  result.duration = ((double)(end - start)) / CLOCKS_PER_SEC;
  result.object = *vm_last_popped(vm);
  return result;
}

static ExecResult exec_interpret(Program* program) {
  clock_t start, end;
  Env* env = env_new();
  start = clock();
  Object evaluated = eval(program, PROGRAM_NODE, env);
  end = clock();

  ExecResult result;
  result.duration = ((double)(end - start)) / CLOCKS_PER_SEC;
  result.object = evaluated;
  return result;
}

static char* get_filename(int argc, char** argv) {
  if (argc > 2) {
    for (int i = 2; i < argc; i++) {
      char* arg = argv[i];
      int len = strlen(arg);
      if (len > 4 && str_is(".mky", &(arg[len - 4]))) {
        return arg;
      }
    }
  }

  puts(COLOR_RED "error: no input supplied" COLOR_RESET);
  exit(EXIT_FAILURE);
}

#define MAX_LINE 512
#define MAX_NUM_LINES 1024

static char* input_from_file(int argc, char** argv) {
  char* filename = get_filename(argc, argv);
  if (!filename)
    return NULL;

  FILE* file = fopen(filename, "r");
  if (!file)
    return NULL;

  int len = 0;
  char line[MAX_LINE];
  char* code = malloc(MAX_LINE * MAX_NUM_LINES);
  for (;;) {
    if (fgets(line, MAX_LINE, file) == NULL)
      break;
    len += strlen(line);
    if (len > MAX_LINE * MAX_NUM_LINES) {
      printf(COLOR_RED
        "error: input exceeded max permitted size of %d" COLOR_RESET,
        MAX_LINE * MAX_NUM_LINES);
      exit(EXIT_FAILURE);
    }
    strcat(code, line);
  }

  return code;
}
