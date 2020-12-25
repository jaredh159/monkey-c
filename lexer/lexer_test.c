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

void test_skips_whitespace()
{
  char t[] = "skips_whitespace";
  Token *tok;
  lexer_set("= =");

  tok = next_token();
  assert(token_is(tok, TOKEN_ASSIGN), "Token type should be ASSIGN", t);

  tok = next_token();
  assert(token_is(tok, TOKEN_ASSIGN), "Token type should be ASSIGN", t);

  tok = next_token();
  assert(token_is(tok, TOKEN_EOF), "Token type should be EOF", t);
}

void test_understands_let()
{
  char t[] = "understands_let";
  lexer_set("let");
  Token *tok = next_token();
  assert(token_is(tok, TOKEN_LET), "Token type should be LET", t);
  assert(token_literal_is(tok, "let"), "Token literal should be let", t);
}

void test_understands_identifiers()
{
  char t[] = "understands_identifiers";
  lexer_set("foo");
  Token *tok = next_token();
  assert(token_is(tok, TOKEN_IDENTIFIER), "Token type should be IDENTIFIER", t);
  assert(token_literal_is(tok, "foo"), "Token literal should be foo", t);
}

void test_realistic_code()
{
  char t[] = "realistic_code";
  char input[] =
      "let five = 5;\n"
      "let ten = 10;\n"
      "\n"
      "let add = fn(x, y) {\n"
      "  x + y;\n"
      "};\n"
      "\n"
      "let result = add(five, ten);\n";

  Token expected[] = {
      {TOKEN_LET, "let"},
      {TOKEN_IDENTIFIER, "five"},
      {TOKEN_ASSIGN, "="},
      {TOKEN_INTEGER, "5"},
      {TOKEN_SEMICOLON, ";"},
      {TOKEN_LET, "let"},
      {TOKEN_IDENTIFIER, "ten"},
      {TOKEN_ASSIGN, "="},
      {TOKEN_INTEGER, "10"},
      {TOKEN_SEMICOLON, ";"},
      {TOKEN_LET, "let"},
      {TOKEN_IDENTIFIER, "add"},
      {TOKEN_ASSIGN, "="},
      {TOKEN_FUNCTION, "fn"},
      {TOKEN_LEFT_PAREN, "("},
      {TOKEN_IDENTIFIER, "x"},
      {TOKEN_COMMA, ","},
      {TOKEN_IDENTIFIER, "y"},
      {TOKEN_RIGHT_PAREN, ")"},
      {TOKEN_LEFT_BRACE, "{"},
      {TOKEN_IDENTIFIER, "x"},
      {TOKEN_PLUS, "+"},
      {TOKEN_IDENTIFIER, "y"},
      {TOKEN_SEMICOLON, ";"},
      {TOKEN_RIGHT_BRACE, "}"},
      {TOKEN_SEMICOLON, ";"},
      {TOKEN_LET, "let"},
      {TOKEN_IDENTIFIER, "result"},
      {TOKEN_ASSIGN, "="},
      {TOKEN_IDENTIFIER, "add"},
      {TOKEN_LEFT_PAREN, "("},
      {TOKEN_IDENTIFIER, "five"},
      {TOKEN_COMMA, ","},
      {TOKEN_IDENTIFIER, "ten"},
      {TOKEN_RIGHT_PAREN, ")"},
      {TOKEN_SEMICOLON, ";"},
      {TOKEN_EOF, ""},
  };
  int expected_len = sizeof(expected) / sizeof(Token);
  lexer_set(input);
  int i;
  Token expected_tok;
  Token *tok;
  char msg[50];
  for (i = 0; i < expected_len; i += 1)
  {
    tok = next_token();
    expected_tok = expected[i];
    sprintf(msg, "Token type should be %s", expected_tok.type);
    assert(token_is(tok, expected_tok.type), msg, t);
    sprintf(msg, "Token literal should be %s", expected_tok.literal);
    assert(token_literal_is(tok, expected_tok.literal), msg, t);
  }
}

int main(void)
{
  test_single_token();
  test_multiple_tokens();
  test_skips_whitespace();
  test_understands_let();
  test_understands_identifiers();
  test_realistic_code();
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
