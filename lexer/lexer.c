#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../token/token.h"

#define MAX_SRC_LEN 4096
#define MAX_STR_LEN 1024
#define MAX_IDENTIFIER_LEN 100

static char input[MAX_SRC_LEN];
static int position = 0;
static int read_position = 0;
static int input_length = 0;
static char ch;
static bool is_letter(char);
static bool is_number(char);
static void read_char(void);
static char peek_char(void);
static void skip_whitespace(void);
static char *read_identifier(void);
static char *read_number();
static char *char_to_str(char);
static int lookup_ident(char *);
static char *read_string(void);

extern void lexer_push(char *pushed_src) {
  int i, j;
  int pushed_length = strlen(pushed_src);
  for (i = input_length, j = 0; i < input_length + pushed_length; i++, j++)
    input[i] = pushed_src[j];
  input[i + 1] = '\0';
  input_length += strlen(pushed_src);
}

extern void lexer_set(char *str) {
  input_length = 0;
  position = 0;
  read_position = 0;
  input[0] = '\0';
  lexer_push(str);
  read_char();
}

extern Token *lexer_next_token() {
  Token *tok;
  skip_whitespace();
  switch (ch) {
    case '"':
      tok = new_token(TOKEN_STRING, read_string());
      break;
    case ':':
      tok = new_token(TOKEN_COLON, ":");
      break;
    case ']':
      tok = new_token(TOKEN_RIGHT_BRACKET, "]");
      break;
    case '[':
      tok = new_token(TOKEN_LEFT_BRACKET, "[");
      break;
    case '-':
      tok = new_token(TOKEN_MINUS, "-");
      break;
    case '/':
      tok = new_token(TOKEN_SLASH, "/");
      break;
    case '*':
      tok = new_token(TOKEN_ASTERISK, "*");
      break;
    case '<':
      tok = new_token(TOKEN_LT, "<");
      break;
    case '>':
      tok = new_token(TOKEN_GT, ">");
      break;
    case '=':
      if (peek_char() == '=') {
        tok = new_token(TOKEN_EQ, "==");
        read_char();
      } else
        tok = new_token(TOKEN_ASSIGN, "=");
      break;
    case '!':
      if (peek_char() == '=') {
        tok = new_token(TOKEN_NOT_EQ, "!=");
        read_char();
      } else
        tok = new_token(TOKEN_BANG, "!");
      break;
    case '+':
      tok = new_token(TOKEN_PLUS, "+");
      break;
    case '(':
      tok = new_token(TOKEN_LEFT_PAREN, "(");
      break;
    case ')':
      tok = new_token(TOKEN_RIGHT_PAREN, ")");
      break;
    case '{':
      tok = new_token(TOKEN_LEFT_BRACE, "{");
      break;
    case '}':
      tok = new_token(TOKEN_RIGHT_BRACE, "}");
      break;
    case ',':
      tok = new_token(TOKEN_COMMA, ",");
      break;
    case ';':
      tok = new_token(TOKEN_SEMICOLON, ";");
      break;
    case 0:
      tok = new_token(TOKEN_EOF, "");
      break;
    default:
      if (is_letter(ch)) {
        char *ident = read_identifier();
        return new_token(lookup_ident(ident), ident);
      } else if (is_number(ch))
        return new_token(TOKEN_INTEGER, read_number());
      else
        tok = new_token(TOKEN_ILLEGAL, char_to_str(ch));
      break;
  }

  read_char();
  return tok;
}

static void read_char() {
  if (read_position >= input_length)
    ch = 0;
  else
    ch = input[read_position];
  position = read_position;
  read_position += 1;
}

static char peek_char() {
  if (read_position >= input_length)
    return 0;
  else
    return input[read_position];
}

static void skip_whitespace() {
  while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') read_char();
}

static bool is_number(char c) {
  return isdigit(c);
}

static bool is_letter(char c) {
  return c == '_' || isalpha(c);
}

static char *read_identifier() {
  char *ident = (char *)malloc(MAX_IDENTIFIER_LEN);
  char *loc = ident;
  while (is_letter(ch)) {
    *loc = ch;
    loc += 1;
    read_char();
  }
  *loc = '\0';
  return ident;
}

static int lookup_ident(char *ident) {
  if (strcmp(ident, "let") == 0)
    return TOKEN_LET;
  if (strcmp(ident, "fn") == 0)
    return TOKEN_FUNCTION;
  if (strcmp(ident, "true") == 0)
    return TOKEN_TRUE;
  if (strcmp(ident, "false") == 0)
    return TOKEN_FALSE;
  if (strcmp(ident, "if") == 0)
    return TOKEN_IF;
  if (strcmp(ident, "else") == 0)
    return TOKEN_ELSE;
  if (strcmp(ident, "return") == 0)
    return TOKEN_RETURN;
  return TOKEN_IDENTIFIER;
}

static char *read_number() {
  char *num = malloc(MAX_IDENTIFIER_LEN);
  char *current = num;
  while (is_number(ch)) {
    *current = ch;
    current += 1;
    read_char();
  }
  *current = '\0';
  return num;
}

static char *read_string(void) {
  char *string = malloc(MAX_STR_LEN);
  int pos = 0;
  while (true) {
    read_char();
    if (ch == '"' || ch == 0) {
      break;
    }
    string[pos] = ch;
    pos += 1;
  }
  string[pos] = '\0';
  return string;
}

static char *char_to_str(char c) {
  char *str = malloc(2);
  str[0] = c;
  str[1] = '\0';
  return str;
}
