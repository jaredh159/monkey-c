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

  if (argv_has_flag('t', argc, argv) || argv_idx("--tokens", argc, argv) != -1)
    repl_start_tokens();
  else if (argv_has_flag('e', argc, argv) ||
           argv_idx("--eval", argc, argv) != -1)
    repl_start_eval();
  else if (argv_has_flag('p', argc, argv) ||
           argv_idx("--parsed", argc, argv) != -1)
    repl_start_parsed_string();
  else
    repl_start();
}
