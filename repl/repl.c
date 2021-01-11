#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../utils/colors.h"
#include "../lexer/lexer.h"
#include "../token/token.h"

#define MAX_LINE_LEN 100

extern void repl_start(void)
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
