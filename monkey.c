#include <stdio.h>
#include "colors.c"
#include "repl/repl.c"

int main(void)
{
  printf(COLOR_MAGENTA "\nWelcome to MONKEY\n" COLOR_RESET);
  printf(COLOR_GREY "Try out the language below...\n\n" COLOR_RESET);
  repl_start();
}
