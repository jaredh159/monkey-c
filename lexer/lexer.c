#include "../token.c"

#include <stdlib.h>
#include <string.h>

#define MAX_SRC_LEN 4096

static char input[MAX_SRC_LEN];
static int position = 0;
static int read_position = 0;
static int input_length = 0;
static char ch;
static void read_char();

extern void lexer_push(char *pushed_src)
{
  int i, j;
  int pushed_length = strlen(pushed_src);
  for (i = input_length, j = 0; i < input_length + pushed_length; i++, j++)
    input[i] = pushed_src[j];
  input[i + 1] = '\0';
  input_length += strlen(pushed_src);
}

extern void lexer_set(char *str)
{
  input_length = 0;
  position = 0;
  read_position = 0;
  input[0] = '\0';
  lexer_push(str);
  read_char();
}

extern Token *next_token()
{
  Token *tok;
  switch (ch)
  {
  case '=':
    tok = new_token(TOKEN_ASSIGN, "=");
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
    break;
  }

  read_char();
  return tok;
}

static void read_char()
{
  if (read_position >= input_length)
    ch = 0;
  else
    ch = input[read_position];
  position = read_position;
  read_position += 1;
}
