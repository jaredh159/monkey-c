#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <stdbool.h>

typedef struct Token {
  int type;
  char *literal;
} Token;

Token *new_token(int type, char *literal);
void print_token(Token *tok);
bool str_is(char *, char *);
bool token_prop_is(char *, char *);
char *token_type_name(int type);

enum TokenType {
  TOKEN_STRING,
  TOKEN_ASSIGN,
  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_BANG,
  TOKEN_ASTERISK,
  TOKEN_SLASH,
  TOKEN_LT,
  TOKEN_GT,
  TOKEN_EQ,
  TOKEN_NOT_EQ,
  TOKEN_LET,
  TOKEN_TRUE,
  TOKEN_FALSE,
  TOKEN_IF,
  TOKEN_ELSE,
  TOKEN_RETURN,
  TOKEN_SEMICOLON,
  TOKEN_IDENTIFIER,
  TOKEN_INTEGER,
  TOKEN_LEFT_PAREN,
  TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE,
  TOKEN_RIGHT_BRACE,
  TOKEN_ILLEGAL,
  TOKEN_COMMA,
  TOKEN_FUNCTION,
  TOKEN_EOF
};

#endif  // __TOKEN_H__
