#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ast/ast.h"
#include "../test/test.h"

typedef struct {
  int type;
  char *ident_val;
  int int_val;
  char *int_literal;
} LitExpTest;

enum LiteralExpressionType {
  LITERAL_TYPE_INT,
  LITERAL_TYPE_IDENT,
  LITERAL_TYPE_BOOL,
};

static LitExpTest zero = {LITERAL_TYPE_INT, "", 0, "0"};
static LitExpTest one = {LITERAL_TYPE_INT, "", 1, "1"};
static LitExpTest two = {LITERAL_TYPE_INT, "", 2, "2"};
static LitExpTest three = {LITERAL_TYPE_INT, "", 3, "3"};
static LitExpTest four = {LITERAL_TYPE_INT, "", 4, "4"};
static LitExpTest five = {LITERAL_TYPE_INT, "", 5, "5"};
static LitExpTest _true = {LITERAL_TYPE_BOOL, "", (int)true, "true"};
static LitExpTest _false = {LITERAL_TYPE_BOOL, "", (int)false, "false"};
static LitExpTest x = {LITERAL_TYPE_IDENT, "x", 0, ""};
static LitExpTest y = {LITERAL_TYPE_IDENT, "y", 0, ""};

void assert_literal_expression(
  Expression *expr, LitExpTest *expected, char *test_name);
void check_parser_errors(char *test_name);
Program *assert_program(
  char *input, int expected_num_statements, char *test_name);
void assert_let_statement(Statement *, char *identifier, char *test_name);
void assert_return_statement(Statement *stmt, char *test_name);
void assert_integer_literal(
  Expression *exp, int value, char *literal, char *test_name);
void assert_identifier(Expression *exp, char *value, char *test_name);
void assert_infix_expression(
  Expression *expr, LitExpTest left, char *op, LitExpTest right, char *t);
void assert_boolean_literal(Expression *exp, bool value, char *test_name);

void test_parses_identifier_expression() {
  char *t = "parses_identifier_expression";
  Program *program = assert_program("foobar;", 1, t);
  Statement *stmt = program->statements->item;
  assert(stmt->type == STATEMENT_EXPRESSION, "statement is expression", t);
  ExpressionStatement *es = get_expression(stmt);
  Expression *exp = es->expression;
  assert_int_is(
    exp->type, EXPRESSION_IDENTIFIER, "expression is identifier", t);
  assert_str_is(
    exp->token_literal, "foobar", "expression token_literal is foobar", t);
  Identifier *ident = exp->node;
  assert_str_is(ident->value, "foobar", "identifier value is foobar", t);
  assert_str_is(
    ident->token->literal, "foobar", "identifier->token->literal is foobar", t);
}

void test_parses_one_let_statement() {
  char *t = "parses_one_let_statement";
  Program *program = assert_program("let x = 5;", 1, t);
  Statement *stmt = program->statements->item;
  assert_let_statement(stmt, "x", t);
}

void test_parses_multiple_let_statements() {
  char *t = "parses_multiple_let_statements";
  Statement *stmt;
  Program *program = assert_program(
    "let x = 5;\n"
    "let y = 10;\n"
    "let foobar = 838383;",
    3, t);

  stmt = program->statements->item;
  assert_let_statement(stmt, "x", t);
  stmt = program->statements->next->item;
  assert_let_statement(stmt, "y", t);
  stmt = program->statements->next->next->item;
  assert_let_statement(stmt, "foobar", t);
}

void test_parses_return_statements() {
  char *t = "parses_return_statements";
  Program *program = assert_program(
    "return 5;\n"
    "return 10;\n"
    "return 993322;\n",
    3, t);
  assert_return_statement(program->statements->item, t);
  assert_return_statement(program->statements->next->item, t);
  assert_return_statement(program->statements->next->next->item, t);
}

void test_parses_integer_literal_expression() {
  char *t = "parses_integer_literal_expression";
  Program *program = assert_program("5;", 1, t);
  Statement *stmt = program->statements->item;
  assert(
    stmt->type == STATEMENT_EXPRESSION, "first statement is expression", t);
  ExpressionStatement *es = get_expression(stmt);
  Expression *exp = es->expression;
  assert_integer_literal(exp, 5, "5", t);
}

void test_parses_boolean_literal_expression() {
  char *t = "parses_boolean_literal_expression";
  Program *program = assert_program("true;", 1, t);
  Statement *stmt = program->statements->item;
  assert(
    stmt->type == STATEMENT_EXPRESSION, "first statement is expression", t);
  ExpressionStatement *es = get_expression(stmt);
  Expression *exp = es->expression;
  assert_boolean_literal(exp, true, t);
}

void test_parses_prefix_expressions() {
  char *t = "parses_prefix_expressions";
  typedef struct {
    char *input;
    char *operator;
    LitExpTest value;
  } PrefixTest;

  PrefixTest tests[] = {{"!true;", "!", _true}, {"!false;", "!", _false},
    {"!5;", "!", five}, {"-15;", "-", {LITERAL_TYPE_INT, "", 15, "15"}}};

  for (int i = 0; i < 2; i++) {
    PrefixTest test = tests[i];
    Program *program = assert_program(test.input, 1, t);
    Statement *stmt = program->statements->item;
    assert(
      stmt->type == STATEMENT_EXPRESSION, "first statement is expression", t);
    ExpressionStatement *es = get_expression(stmt);
    Expression *exp = es->expression;
    assert_int_is(exp->type, EXPRESSION_PREFIX, "expression type is PREFIX", t);
    PrefixExpression *prefix = exp->node;
    assert_str_is(
      test.operator, prefix->operator, ss("operator is %s", test.operator), t);
    assert_literal_expression(prefix->right, &test.value, t);
  }
}

void test_parses_infix_expressions() {
  char *t = "parses_infix_expressions";
  typedef struct {
    char *input;
    LitExpTest left;
    char *operator;
    LitExpTest right;
  } InfixTest;

  InfixTest tests[] = {
    {"true == true", _true, "==", _true},
    {"true != false", _true, "!=", _false},
    {"false == false", _false, "==", _false},
    {"5 + 5;", five, "+", five},
    {"5 - 5;", five, "-", five},
    {"5 * 5;", five, "*", five},
    {"5 / 5;", five, "/", five},
    {"5 > 5;", five, ">", five},
    {"5 < 5;", five, "<", five},
    {"5 == 5;", five, "==", five},
    {"5 != 5;", five, "!=", five},
  };

  int num_tests = sizeof(tests) / sizeof(InfixTest);
  for (int i = 0; i < num_tests; i++) {
    InfixTest test = tests[i];
    Program *program = assert_program(test.input, 1, t);
    Statement *stmt = program->statements->item;
    assert(
      stmt->type == STATEMENT_EXPRESSION, "first statement is expression", t);
    ExpressionStatement *es = get_expression(stmt);
    Expression *exp = es->expression;
    assert_infix_expression(exp, test.left, test.operator, test.right, t);
  }
}

void test_operator_precedence_parsing() {
  char *t = "operator_precedence_parsing";
  typedef struct {
    char *input;
    char *expected;
  } PrecedenceTest;

  PrecedenceTest tests[] = {
    {
      "3 + 4; -5 * 5",
      "(3 + 4)((-5) * 5)",
    },
    {
      "1 + (2 + 3) + 4",
      "((1 + (2 + 3)) + 4)",
    },
    {
      "(5 + 5) * 2",
      "((5 + 5) * 2)",
    },
    {
      "2 / (5 + 5)",
      "(2 / (5 + 5))",
    },
    {
      "-(5 + 5)",
      "(-(5 + 5))",
    },
    {
      "!(true == true)",
      "(!(true == true))",
    },
    {
      "true",
      "true",
    },
    {
      "false",
      "false",
    },
    {
      "3 > 5 == false",
      "((3 > 5) == false)",
    },
    {
      "3 > 5 == true",
      "((3 > 5) == true)",
    },
    {
      "-a * b",
      "((-a) * b)",
    },
    {
      "!-a",
      "(!(-a))",
    },
    {
      "a + b + c",
      "((a + b) + c)",
    },
    {
      "a * b * c",
      "((a * b) * c)",
    },
    {
      "a * b / c",
      "((a * b) / c)",
    },
    {
      "a + b / c",
      "(a + (b / c))",
    },
    {
      "a + b * c + d / e - f",
      "(((a + (b * c)) + (d / e)) - f)",
    },
    {
      "5 > 4 == 3 < 4",
      "((5 > 4) == (3 < 4))",
    },
    {
      "5 < 4 != 3 > 4",
      "((5 < 4) != (3 > 4))",
    },
    {
      "3 + 4 * 5 == 3 * 1 + 4 * 5",
      "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))",
    },
    {
      "a + add(b * c) + d",
      "((a + add((b * c))) + d)",
    },
    {
      "add(a, b, 1, 2 * 3, 4 + 5, add(6, 7 * 8))",
      "add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)))",
    },
    {
      "add(a + b + c * d / f + g)",
      "add((((a + b) + ((c * d) / f)) + g))",
    },
    {
      "a * [1, 2, 3, 4][b * c] * d",
      "((a * ([1, 2, 3, 4][(b * c)])) * d)",
    },
    {
      "add(a * b[2], b[1], 2 * [1, 2][1])",
      "add((a * (b[2])), (b[1]), (2 * ([1, 2][1])))",
    },
  };
  int num_tests = sizeof(tests) / sizeof(PrecedenceTest);

  for (int i = 0; i < num_tests; i++) {
    PrecedenceTest test = tests[i];
    Program *program = assert_program(test.input, i == 0 ? 2 : 1, t);
    char *actual = program_string(program);
    assert_str_is(test.expected, actual, "program string is correct", t);
  }
}

void test_parses_if_expression() {
  char *t = "parses_if_expression";
  Program *program = assert_program("if (x < y) { x }", 1, t);
  Statement *stmt = program->statements->item;
  assert(
    stmt->type == STATEMENT_EXPRESSION, "first statement is expression", t);
  ExpressionStatement *es = get_expression(stmt);
  Expression *exp = es->expression;
  assert(exp->type == EXPRESSION_IF, "expression is IF", t);
  IfExpression *if_exp = exp->node;
  assert_infix_expression(if_exp->condition, x, "<", y, t);
  assert_int_is(1, list_count(if_exp->consequence->statements),
    "consequence has 1 statement", t);
  Statement *consequence = if_exp->consequence->statements->item;
  assert(consequence->type == STATEMENT_EXPRESSION,
    "first consequence statement is expression", t);
  ExpressionStatement *cq_exp = get_expression(consequence);
  assert_identifier(cq_exp->expression, "x", t);
  assert(if_exp->alternative == NULL, "alternative is NULL", t);
}

void test_parses_if_else_expression() {
  char *t = "parses_if_else_expression";
  Program *program = assert_program("if (x < y) { x } else { y }", 1, t);
  Statement *stmt = program->statements->item;
  assert(
    stmt->type == STATEMENT_EXPRESSION, "first statement is expression", t);
  ExpressionStatement *es = get_expression(stmt);
  Expression *exp = es->expression;
  assert(exp->type == EXPRESSION_IF, "expression is IF", t);
  IfExpression *if_exp = exp->node;
  LitExpTest x = {LITERAL_TYPE_IDENT, "x", 0, ""};
  LitExpTest y = {LITERAL_TYPE_IDENT, "y", 0, ""};
  assert_infix_expression(if_exp->condition, x, "<", y, t);

  // test consequence
  assert_int_is(1, list_count(if_exp->consequence->statements),
    "consequence has 1 statement", t);
  Statement *consequence = if_exp->consequence->statements->item;
  assert(consequence->type == STATEMENT_EXPRESSION,
    "first consequence statement is expression", t);
  ExpressionStatement *cq_exp = get_expression(consequence);
  assert_identifier(cq_exp->expression, "x", t);

  // test alternative
  assert_int_is(1, list_count(if_exp->alternative->statements),
    "alternative has 1 statement", t);
  Statement *alternative = if_exp->alternative->statements->item;
  assert(alternative->type == STATEMENT_EXPRESSION,
    "first alternative statement is expression", t);
  ExpressionStatement *at_exp = get_expression(alternative);
  assert_identifier(at_exp->expression, "y", t);
}

void test_parses_function_literal() {
  char *t = "parses_function_literal";
  Program *program = assert_program("fn(x, y) { x + y; }", 1, t);
  Statement *stmt = program->statements->item;
  assert(
    stmt->type == STATEMENT_EXPRESSION, "first statement is expression", t);
  ExpressionStatement *es = get_expression(stmt);
  Expression *exp = es->expression;
  assert(exp->type == EXPRESSION_FUNCTION_LITERAL,
    "expression is function literal", t);
  FunctionLiteral *fn = exp->node;

  assert_int_is(2, list_count(fn->parameters), "there are TWO params", t);
  Identifier *param1 = fn->parameters->item;
  assert_str_is("x", param1->token->literal, "first param is x", t);
  assert_str_is("x", param1->value, "param1 identifier is x", t);
  Identifier *param2 = fn->parameters->next->item;
  assert_str_is("y", param2->token->literal, "second param is y", t);
  assert_str_is("y", param2->value, "param2 identifier is x", t);

  assert_int_is(
    1, list_count(fn->body->statements), "one statement in body", t);
  ExpressionStatement *body_stmt = get_expression(fn->body->statements->item);
  assert_infix_expression(body_stmt->expression, x, "+", y, t);
}

void test_function_parameter_parsing() {
  char *t = "function_parameter_parsing";
  typedef struct {
    char *input;
    int num_params;
    char *params[3];
  } ParamTest;

  ParamTest tests[] = {{"fn() {}", 0, {"", "", ""}},
    {"fn(x) {}", 1, {"x", "", ""}}, {"fn(x, y, z) {}", 3, {"x", "y", "z"}}};
  int num_tests = sizeof tests / sizeof(ParamTest);

  for (int i = 0; i < num_tests; i++) {
    ParamTest test = tests[i];
    Program *program = assert_program(test.input, 1, t);
    Statement *stmt = program->statements->item;
    ExpressionStatement *es = get_expression(stmt);
    FunctionLiteral *fn = es->expression->node;

    assert_int_is(
      test.num_params, list_count(fn->parameters), "num params correct", t);
    if (test.num_params == 0)
      continue;

    List *current = fn->parameters;
    for (int j = 0; j < test.num_params; j++, current = current->next) {
      Identifier *param = current->item;
      assert_str_is(test.params[j], param->value, "param is correct", t);
    }
  }
}

void test_let_statements() {
  char *t = "let_statements";
  typedef struct {
    char *input;
    char *expected_identifier;
    LitExpTest expected_value;
  } LetTest;

  LetTest tests[] = {
    {"let x = 5;", "x", five},
    {"let y = true;", "y", _true},
    {"let foobar = y;", "foobar", y},
  };

  int num_tests = sizeof tests / sizeof(LetTest);
  for (int i = 0; i < num_tests; i++) {
    LetTest test = tests[i];
    Program *program = assert_program(test.input, 1, t);
    Statement *stmt = program->statements->item;
    assert_let_statement(stmt, test.expected_identifier, t);
    LetStatement *ls = get_let(stmt);
    assert_literal_expression(ls->value, &test.expected_value, t);
  }
}

void test_call_expression_parsing() {
  char *t = "call_expression_parsing";
  Program *program = assert_program("add(1, 2 * 3, 4 + 5)", 1, t);
  ExpressionStatement *es = get_expression(program->statements->item);
  assert(es->expression->type == EXPRESSION_CALL,
    "expression is call expression", t);
  CallExpression *ce = es->expression->node;
  assert_identifier(ce->fn, "add", t);
  assert_int_is(3, list_count(ce->arguments), "correct num args", t);
  assert_literal_expression(ce->arguments->item, &one, t);
  assert_infix_expression(ce->arguments->next->item, two, "*", three, t);
  assert_infix_expression(ce->arguments->next->next->item, four, "+", five, t);
}

void test_string_literal_expression(void) {
  char *t = "string_literal_expression";
  Program *program = assert_program("\"hello world\";", 1, t);
  ExpressionStatement *es = get_expression(program->statements->item);
  assert(es->expression->type == EXPRESSION_STRING_LITERAL,
    "expression is string literal expression", t);
  StringLiteral *str = es->expression->node;
  assert_str_is("hello world", str->value, "string literal correct", t);
}

void test_array_literal(void) {
  char *t = "array_literal";
  Program *program = assert_program("[1, 2 * 2, 3 + 3]", 1, t);
  ExpressionStatement *es = get_expression(program->statements->item);
  assert(es->expression->type == EXPRESSION_ARRAY_LITERAL,
    "expression is array literal expression", t);
  ArrayLiteral *al = es->expression->node;
  assert_int_is(3, list_count(al->elements), "array has 3 items", t);
  assert_integer_literal(al->elements->item, 1, "1", t);
  assert_infix_expression(al->elements->next->item, two, "*", two, t);
  assert_infix_expression(al->elements->next->next->item, three, "+", three, t);
}

void test_empty_array_literal(void) {
  char *t = "empty_array_literal";
  Program *program = assert_program("[]", 1, t);
  ExpressionStatement *es = get_expression(program->statements->item);
  assert(es->expression->type == EXPRESSION_ARRAY_LITERAL,
    "expression is array literal expression", t);
  ArrayLiteral *al = es->expression->node;
  assert_int_is(0, list_count(al->elements), "array has 3 items", t);
}

void test_parsing_index_expressions(void) {
  char *t = "parsing_index_expressions";
  Program *program = assert_program("myArray[1 + 1]", 1, t);
  ExpressionStatement *es = get_expression(program->statements->item);
  assert(es->expression->type == EXPRESSION_INDEX, "expression is index", t);
  IndexExpression *ie_exp = es->expression->node;
  assert_identifier(ie_exp->left, "myArray", t);
  assert_infix_expression(ie_exp->index, one, "+", one, t);
}

void test_parsing_empty_hash_literal(void) {
  char *t = "parsing_empty_hash_literal";
  Program *program = assert_program("{}", 1, t);
  ExpressionStatement *es = get_expression(program->statements->item);
  assert(es->expression->type == EXPRESSION_HASH_LITERAL,
    "expression is hash literal", t);
  HashLiteralExpression *hl_exp = es->expression->node;
  assert_int_is(0, list_count(hl_exp->pairs), "hash has 0 pairs", t);
}

void test_parsing_hash_literals_with_expressions(void) {
  char *t = "parsing_hash_literals_with_expressions";
  Program *program =
    assert_program("{\"one\": 0 + 1, \"two\": 5 - 1, \"three\": 4 / 2}", 1, t);
  ExpressionStatement *es = get_expression(program->statements->item);
  assert(es->expression->type == EXPRESSION_HASH_LITERAL,
    "expression is hash literal", t);
  HashLiteralExpression *hl_exp = es->expression->node;
  assert_int_is(3, list_count(hl_exp->pairs), "hash has 3 pairs", t);
  List *pairs = hl_exp->pairs;

  HashLiteralPair *pair1 = pairs->item;
  assert_infix_expression(pair1->value, zero, "+", one, t);

  HashLiteralPair *pair2 = pairs->next->item;
  assert_infix_expression(pair2->value, five, "-", one, t);

  HashLiteralPair *pair3 = pairs->next->next->item;
  assert_infix_expression(pair3->value, four, "/", two, t);
}

void test_parsing_hash_literals(void) {
  char *t = "parsing_hash_literals";
  Program *program =
    assert_program("{\"one\": 1, \"two\": 2, \"three\": 3}", 1, t);
  ExpressionStatement *es = get_expression(program->statements->item);
  assert(es->expression->type == EXPRESSION_HASH_LITERAL,
    "expression is hash literal", t);
  HashLiteralExpression *hl_exp = es->expression->node;
  assert_int_is(3, list_count(hl_exp->pairs), "hash has 3 pairs", t);
  List *pairs = hl_exp->pairs;

  HashLiteralPair *pair1 = pairs->item;
  assert(
    pair1->key->type == EXPRESSION_STRING_LITERAL, "key is str literal", t);
  StringLiteral *str = pair1->key->node;
  assert_str_is("one", str->value, "key string literal correct", t);
  assert_integer_literal(pair1->value, 1, "1", t);

  HashLiteralPair *pair2 = pairs->next->item;
  assert(
    pair2->key->type == EXPRESSION_STRING_LITERAL, "key is str literal", t);
  str = pair2->key->node;
  assert_str_is("two", str->value, "key string literal correct", t);
  assert_integer_literal(pair2->value, 2, "2", t);

  HashLiteralPair *pair3 = pairs->next->next->item;
  assert(
    pair3->key->type == EXPRESSION_STRING_LITERAL, "key is str literal", t);
  str = pair3->key->node;
  assert_str_is("three", str->value, "key string literal correct", t);
  assert_integer_literal(pair3->value, 3, "3", t);
}

int main(int argc, char **argv) {
  pass_argv(argc, argv);
  test_parsing_hash_literals();
  test_parsing_hash_literals_with_expressions();
  test_parsing_empty_hash_literal();
  test_parsing_index_expressions();
  test_array_literal();
  test_empty_array_literal();
  test_string_literal_expression();
  test_let_statements();
  test_call_expression_parsing();
  test_function_parameter_parsing();
  test_parses_function_literal();
  test_parses_if_expression();
  test_parses_if_else_expression();
  test_parses_boolean_literal_expression();
  test_operator_precedence_parsing();
  test_parses_infix_expressions();
  test_parses_prefix_expressions();
  test_parses_identifier_expression();
  test_parses_return_statements();
  test_parses_one_let_statement();
  test_parses_multiple_let_statements();
  test_parses_integer_literal_expression();
  printf("\n");
  return 0;
}

void assert_let_statement(Statement *stmt, char *identifier, char *test_name) {
  assert_str_is(stmt->token_literal, "let",
    "statement->token_literal is \"let\"", test_name);

  assert(stmt->type == STATEMENT_LET, "statement TYPE is `let`", test_name);

  char msg[50];
  sprintf(msg, "identifier is \"%s\"", identifier);

  assert_str_is(get_let(stmt)->name->value, identifier, msg, test_name);
}

void assert_return_statement(Statement *stmt, char *test_name) {
  assert_str_is(stmt->token_literal, "return",
    "statement->token_literal is \"return\"", test_name);

  assert(
    stmt->type == STATEMENT_RETURN, "statement TYPE is `return`", test_name);
}

void check_parser_errors(char *test_name) {
  if (!parser_has_error())
    return;

  parser_print_errors();
  char msg[50];
  sprintf(msg, "parser had %i errors\n", parser_num_errors());
  fail(msg, test_name);
}

void assert_integer_literal(
  Expression *exp, int value, char *literal, char *test_name) {
  char msg[50];
  assert_int_is(exp->type, EXPRESSION_INTEGER_LITERAL,
    "expression is integer literal", test_name);
  IntegerLiteral *int_literal = exp->node;

  sprintf(msg, "int literal is int %d", value);
  assert_int_is(value, int_literal->value, msg, test_name);

  sprintf(msg, "int_literal->token->literal is \"%s\"", literal);
  assert_str_is(literal, int_literal->token->literal, msg, test_name);
}

void assert_boolean_literal(Expression *exp, bool value, char *test_name) {
  assert_int_is(exp->type, EXPRESSION_BOOLEAN_LITERAL,
    "expression is boolean literal", test_name);
  BooleanLiteral *bool_literal = exp->node;
  assert_int_is(
    (int)value, bool_literal->value, "boolean is correct", test_name);
  assert_str_is(value ? "true" : "false", bool_literal->token->literal,
    "boolean token literal correct", test_name);
}

void assert_identifier(Expression *exp, char *value, char *test_name) {
  assert_int_is(
    EXPRESSION_IDENTIFIER, exp->type, "expression is identifier", test_name);
  Identifier *ident = exp->node;
  assert_str_is(value, ident->value, "identifier value is correct", test_name);
  assert_str_is(value, ident->token->literal,
    "identifier token literal is correct", test_name);
}

void assert_literal_expression(
  Expression *expr, LitExpTest *expected, char *test_name) {
  switch (expected->type) {
    case LITERAL_TYPE_BOOL:
      assert_boolean_literal(expr, (bool)expected->int_val, test_name);
      return;
    case LITERAL_TYPE_INT:
      assert_integer_literal(
        expr, expected->int_val, expected->int_literal, test_name);
      return;
    case LITERAL_TYPE_IDENT:
      assert_identifier(expr, expected->ident_val, test_name);
      return;
  }
  fail("literal expression type not handled", test_name);
}

void assert_infix_expression(Expression *expr, LitExpTest left, char *operand,
  LitExpTest right, char *test_name) {
  assert_int_is(EXPRESSION_INFIX, expr->type, "expression is infix", test_name);
  InfixExpression *infix = expr->node;
  assert_literal_expression(infix->left, &left, test_name);
  assert_str_is(operand, infix->operator, "operand is correct", test_name);
  assert_literal_expression(infix->right, &right, test_name);
}

Program *assert_program(
  char *input, int expected_num_statements, char *test_name) {
  Program *program = parse_program(input);

  if (program == NULL)
    fail("parse_program() returned NULL", test_name);

  check_parser_errors(test_name);
  assert_int_is(expected_num_statements, list_count(program->statements),
    si("program has %d statements", expected_num_statements), test_name);

  return program;
}
