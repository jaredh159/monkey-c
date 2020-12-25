#include "lexer.c"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../test-utils.c"

bool token_is(Token *token, char *type);
bool token_literal_is(Token *token, char *literal);
void assert(bool, char *, char *);

void test_single_token()
{
  lexer_set("=");
  Token *token = next_token();
  assert(token_is(token, TOKEN_ASSIGN), "Token should be =", "single_token");
}

void test_multiple_tokens()
{
  char t[] = "multiple_tokens";
  Token *tok;
  lexer_set("=+(){},;");

  tok = next_token();
  assert(token_is(tok, TOKEN_ASSIGN), "Token type should be ASSIGN", t);
  assert(token_literal_is(tok, "="), "Token literal should be =", t);

  tok = next_token();
  assert(token_is(tok, TOKEN_PLUS), "Token type should be PLUS", t);
  assert(token_literal_is(tok, "+"), "Token literal should be +", t);

  tok = next_token();
  assert(token_is(tok, TOKEN_LEFT_PAREN), "Token type should be LEFT_PAREN", t);
  assert(token_literal_is(tok, "("), "Token literal should be (", t);

  tok = next_token();
  assert(token_is(tok, TOKEN_RIGHT_PAREN), "Token type should be RIGHT_PAREN", t);
  assert(token_literal_is(tok, ")"), "Token literal should be )", t);

  tok = next_token();
  assert(token_is(tok, TOKEN_LEFT_BRACE), "Token type should be LEFT_BRACE", t);
  assert(token_literal_is(tok, "{"), "Token literal should be {", t);

  tok = next_token();
  assert(token_is(tok, TOKEN_RIGHT_BRACE), "Token type should be RIGHT_BRACE", t);
  assert(token_literal_is(tok, "}"), "Token literal should be }", t);

  tok = next_token();
  assert(token_is(tok, TOKEN_COMMA), "Token type should be COMMA", t);
  assert(token_literal_is(tok, ","), "Token literal should be ,", t);

  tok = next_token();
  assert(token_is(tok, TOKEN_SEMICOLON), "Token type should be SEMICOLON", t);
  assert(token_literal_is(tok, ";"), "Token literal should be ;", t);

  tok = next_token();
  assert(token_is(tok, TOKEN_EOF), "Token type should be EOF", t);
  assert(token_literal_is(tok, ""), "Token literal should be <empty>", t);
}

int main(void)
{
  test_single_token();
  test_multiple_tokens();
  return 0;
}

bool token_literal_is(Token *token, char *literal)
{
  return strcmp(token->literal, literal) == 0;
}

bool token_is(Token *token, char *type)
{
  return strcmp(token->type, type) == 0;
}

void assert(bool predicate, char *msg, char *test_name)
{
  if (!predicate)
  {
    printf(COLOR_RED "X %s\n" COLOR_RESET, msg);
    exit(1);
  }
  printf(COLOR_GREEN "√" COLOR_RESET COLOR_GREY " %s: %s\n" COLOR_RESET, test_name, msg);
}
