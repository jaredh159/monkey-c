#include "token.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../utils/colors.h"

Token *new_token(int type, char *literal) {
  Token *token = (Token *)malloc(sizeof(Token));
  token->type = type;
  token->literal = literal;
  return token;
}

void print_token(Token *tok) {
  printf(COLOR_GREY "  -> { type: %s, literal: \"%s\" }\n" COLOR_RESET,
    token_type_name(tok->type), tok->literal);
}

bool str_is(char *str, char *compare) {
  return strcmp(str, compare) == 0;
}

bool token_prop_is(char *prop, char *compare) {
  return str_is(prop, compare);
}

char *token_type_name(int type) {
  switch (type) {
    case TOKEN_ASSIGN:
      return "ASSIGN";
    case TOKEN_PLUS:
      return "PLUS";
    case TOKEN_MINUS:
      return "MINUS";
    case TOKEN_BANG:
      return "BANG";
    case TOKEN_ASTERISK:
      return "ASTERISK";
    case TOKEN_SLASH:
      return "SLASH";
    case TOKEN_LT:
      return "LT";
    case TOKEN_GT:
      return "GT";
    case TOKEN_EQ:
      return "EQ";
    case TOKEN_NOT_EQ:
      return "NOT_EQ";
    case TOKEN_LET:
      return "LET";
    case TOKEN_TRUE:
      return "TRUE";
    case TOKEN_FALSE:
      return "FALSE";
    case TOKEN_IF:
      return "IF";
    case TOKEN_ELSE:
      return "ELSE";
    case TOKEN_RETURN:
      return "RETURN";
    case TOKEN_SEMICOLON:
      return "SEMICOLON";
    case TOKEN_IDENTIFIER:
      return "IDENTIFIER";
    case TOKEN_INTEGER:
      return "INTEGER";
    case TOKEN_LEFT_PAREN:
      return "LEFT_PAREN";
    case TOKEN_RIGHT_PAREN:
      return "RIGHT_PAREN";
    case TOKEN_LEFT_BRACE:
      return "LEFT_BRACE";
    case TOKEN_RIGHT_BRACE:
      return "RIGHT_BRACE";
    case TOKEN_ILLEGAL:
      return "ILLEGAL";
    case TOKEN_COMMA:
      return "COMMA";
    case TOKEN_FUNCTION:
      return "FUNCTION";
    case TOKEN_EOF:
      return "EOF";
  }
  return "UNKNOWN_TOKEN";
}
