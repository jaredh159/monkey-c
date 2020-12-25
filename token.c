#include <stdio.h>
#include <stdlib.h>
#include "test-utils.c"

#define TOKEN_ILLEGAL "ILLEGAL"
#define TOKEN_EOF "EOF"
#define TOKEN_IDENTIFIER "IDENTIFIER"
#define TOKEN_INTEGER "INTEGER"
#define TOKEN_ASSIGN "ASSIGN"
#define TOKEN_PLUS "PLUS"
#define TOKEN_COMMA "COMMA"
#define TOKEN_SEMICOLON "SEMICOLON"
#define TOKEN_LEFT_PAREN "LEFT_PAREN"
#define TOKEN_RIGHT_PAREN "RIGHT_PAREN"
#define TOKEN_LEFT_BRACE "LEFT_BRACE"
#define TOKEN_RIGHT_BRACE "RIGHT_BRACE"
#define TOKEN_FUNCTION "FUNCTION"
#define TOKEN_LET "LET"

typedef struct Token
{
  char *type;
  char *literal;
} Token;

extern Token *new_token(char *type, char *literal)
{
  Token *token = (Token *)malloc(sizeof(Token));
  token->type = type;
  token->literal = literal;
  return token;
}

extern void print_token(Token *tok)
{
  printf(
      COLOR_GREY "  -> { type: %s, literal: \"%s\" }\n" COLOR_RESET,
      tok->type,
      tok->literal);
}
