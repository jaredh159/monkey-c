#ifndef __ARGV_H__
#define __ARGV_H__

#include <stdbool.h>

bool argv_has_flag(char flag, int argc, char *argv[]);
int argv_idx(char *needle, int argc, char *argv[]);
int argv_int_opt(char prefix, int argc, char *argv[]);

#endif  // __ARGV_H__
