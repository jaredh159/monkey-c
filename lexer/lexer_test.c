#include "lexer.c"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../test-utils.c"

bool token_is(Token *token, char *type);
bool token_literal_is(Token *token, char *literal);
void assert(bool, char *, char *);
void assert_lexing(char *, Token[], int, char *);

void test_single_token()
{
  Token expected[] = {{TOKEN_ASSIGN, "="}};
  assert_lexing("=", expected, 1, "single_token");
}

void test_multiple_tokens()
{
  Token expected[] = {
      {TOKEN_ASSIGN, "="},
      {TOKEN_PLUS, "+"},
      {TOKEN_LEFT_PAREN, "("},
      {TOKEN_RIGHT_PAREN, ")"},
      {TOKEN_LEFT_BRACE, "{"},
      {TOKEN_RIGHT_BRACE, "}"},
      {TOKEN_COMMA, ","},
      {TOKEN_SEMICOLON, ";"},
      {TOKEN_EOF, ""},
  };
  assert_lexing("=+(){},;", expected, 9, "multiple_tokens");
}

void test_skips_whitespace()
{
  Token expected[] = {
      {TOKEN_ASSIGN, "="},
      {TOKEN_ASSIGN, "="},
      {TOKEN_EOF, ""},
  };
  assert_lexing("= =", expected, 3, "skips_whitespace");
}

void test_realistic_code()
{
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
  assert_lexing(input, expected, sizeof(expected) / sizeof(Token), "realistic_code");
}

int main(void)
{
  test_single_token();
  test_multiple_tokens();
  test_skips_whitespace();
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

void assert_lexing(char *input, Token expected_tokens[], int num_expected, char *test_name)
{
  int i;
  Token expected;
  Token *actual;
  char msg[50];

  lexer_set(input);

  for (i = 0; i < num_expected; i += 1)
  {
    actual = next_token();
    expected = expected_tokens[i];
    sprintf(msg, "Token type should be %s", expected.type);
    assert(token_is(actual, expected.type), msg, test_name);
    sprintf(msg, "Token literal should be %s", expected.literal);
    assert(token_literal_is(actual, expected.literal), msg, test_name);
  }
}
