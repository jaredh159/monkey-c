#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdbool.h>
#include "../ast/ast.h"

Program *parse_program(char *input);
void parser_push_error(char *error_msg);
bool parser_has_error();
int parser_num_errors();
void parser_print_errors();

#endif // __PARSER_H__
