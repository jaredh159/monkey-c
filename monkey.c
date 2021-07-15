#include <stdio.h>
#include <stdlib.h>
#include "repl/repl.h"
#include "run/run.h"
#include "utils/argv.h"
#include "utils/colors.h"

int main(int argc, char **argv) {
  if (argv_idx("run", argc, argv) == 1) {
    run(argc, argv);
    exit(EXIT_SUCCESS);
  }

  printf(COLOR_MAGENTA "\nWelcome to MONKEY\n" COLOR_RESET);
  printf(COLOR_GREY "Try out the language below...\n\n" COLOR_RESET);

  if (argv_has_opt('t', "--tokens", argc, argv))
    repl_start_tokens();
  else if (argv_has_opt('e', "--eval", argc, argv))
    repl_start_eval();
  else if (argv_has_opt('p', "--parsed", argc, argv))
    repl_start_parsed_string();
  else
    repl_start();
}
