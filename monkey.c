#include <stdio.h>
#include "colors.h"
#include "repl/repl.h"

int main(void)
{
  printf(COLOR_MAGENTA "\nWelcome to MONKEY\n" COLOR_RESET);
  printf(COLOR_GREY "Try out the language below...\n\n" COLOR_RESET);
  repl_start();
}
