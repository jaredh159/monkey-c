#include <stdio.h>
#include "repl/repl.h"
#include "utils/argv.h"
#include "utils/colors.h"

int main(int argc, char **argv) {
  printf(COLOR_MAGENTA "\nWelcome to MONKEY\n" COLOR_RESET);
  printf(COLOR_GREY "Try out the language below...\n\n" COLOR_RESET);

  if (argv_has_flag('t', argc, argv) || argv_idx("--tokens", argc, argv) != -1)
    repl_start_tokens();
  else if (argv_has_flag('p', argc, argv) ||
           argv_idx("--parsed", argc, argv) != -1)
    repl_start_parsed_string();
  else
    repl_start();
}
