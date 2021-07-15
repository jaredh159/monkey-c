#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../code/code.h"
#include "../compiler/compiler.h"
#include "../compiler/symbol_table.h"
#include "../evaluator/evaluator.h"
#include "../lexer/lexer.h"
#include "../object/object.h"
#include "../parser/parser.h"
#include "../token/token.h"
#include "../utils/argv.h"

static int append(char* json, const char* append);

char* wasm_eval(char* program_input) {
  Program* program = parse_program(program_input);
  if (parser_num_errors() > 0) {
    return "ERROR: parser errors found";
  }
  Env* env = env_new();
  Object evaluated = eval(program, PROGRAM_NODE, env);
  return object_inspect(evaluated);
}

char* wasm_tokens(char* program_input) {
  lexer_set(program_input);
  int pos = 0;
  char* json = calloc(4096, 1);
  json[pos++] = '[';
  Token* token = lexer_next_token();
  while (token->type != TOKEN_EOF) {
    pos += append(json, "{\"type\":\"");
    char* type_name = token_type_name(token->type);
    pos += append(json, type_name);
    pos += append(json, "\",\"literal\":\"");
    pos += append(json, token->literal);
    pos += append(json, "\"},");
    token = lexer_next_token();
  }
  json[pos - 1] = ']';
  json[pos] = '\n';
  return json;
}

static int append(char* json, const char* append) {
  strcat(json, append);
  return strlen(append);
}
