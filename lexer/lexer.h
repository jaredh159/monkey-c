#ifndef __LEXER_H__
#define __LEXER_H__

#include "../token/token.h"

void lexer_push(char *pushed_src);
void lexer_set(char *str);
Token *lexer_next_token();

#endif  // __LEXER_H__
