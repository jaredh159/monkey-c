#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <stdbool.h>

typedef struct Token
{
  char *type;
  char *literal;
} Token;

Token *new_token(char *type, char *literal);

void print_token(Token *tok);
bool str_is(char *, char *);
bool token_prop_is(char *, char *);

/* operators */
#define TOKEN_ASSIGN "ASSIGN"
#define TOKEN_PLUS "PLUS"
#define TOKEN_MINUS "MINUS"
#define TOKEN_BANG "BANG"
#define TOKEN_ASTERISK "ASTERISK"
#define TOKEN_SLASH "SLASH"

/* comparison */
#define TOKEN_LT "LT"
#define TOKEN_GT "GT"
#define TOKEN_EQ "EQ"
#define TOKEN_NOT_EQ "NOT_EQ"

/* keywords */
#define TOKEN_FUNCTION "FUNCTION"
#define TOKEN_LET "LET"
#define TOKEN_TRUE "TRUE"
#define TOKEN_FALSE "FALSE"
#define TOKEN_IF "IF"
#define TOKEN_ELSE "ELSE"
#define TOKEN_RETURN "RETURN"

/* delimiters */
#define TOKEN_COMMA "COMMA"
#define TOKEN_SEMICOLON "SEMICOLON"

/* identifiers + literals */
#define TOKEN_INTEGER "INTEGER"
#define TOKEN_IDENTIFIER "IDENTIFIER"

/* ¯\_(ツ)_/¯ */
#define TOKEN_BANG "BANG"
#define TOKEN_LEFT_PAREN "LEFT_PAREN"
#define TOKEN_RIGHT_PAREN "RIGHT_PAREN"
#define TOKEN_LEFT_BRACE "LEFT_BRACE"
#define TOKEN_RIGHT_BRACE "RIGHT_BRACE"

/* utility */
#define TOKEN_ILLEGAL "ILLEGAL"
#define TOKEN_EOF "EOF"

#endif // __TOKEN_H__
