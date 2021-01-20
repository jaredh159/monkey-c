#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../utils/colors.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../token/token.h"

#define MAX_LINE_LEN 100

void repl_start_parsed_string(void)
{
  int num_chars;
  size_t bufsize = MAX_LINE_LEN + 1;
  char *buffer = (char *)malloc(bufsize * sizeof(char));

  do
  {
    printf(COLOR_CYAN ">> " COLOR_RESET);
    num_chars = getline(&buffer, &bufsize, stdin);
    if (num_chars > 1)
    {
      Program *program = parse_program(buffer);
      if (parser_num_errors() > 0)
      {
        parser_print_errors();
        continue;
      }
      printf("%s\n", program_string(program));
    }
  } while (num_chars != EOF);
  printf("\n");
}

void repl_start_tokens(void)
{
  Token *tok;
  ssize_t num_chars = 0;
  size_t bufsize = MAX_LINE_LEN + 1;
  char *buffer = (char *)malloc(bufsize * sizeof(char));

  do
  {
    printf(COLOR_CYAN ">> " COLOR_RESET);
    num_chars = getline(&buffer, &bufsize, stdin);
    if (num_chars > 1)
    {
      lexer_set(buffer);
      for (tok = lexer_next_token(); tok->type != TOKEN_EOF; tok = lexer_next_token())
        print_token(tok);
      printf("\n");
    }
  } while (num_chars != EOF);
  printf("\n");
}
