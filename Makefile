FLAGS = -Wall -O -W -pedantic
monkey:
	clang -o .bin/monkey monkey.c repl/repl.c token/token.c lexer/lexer.c $(FLAGS)

test_parser:
	clang -o .bin/test_parser parser/parser_test.c parser/parser.c test/test.c lexer/lexer.c token/token.c ast/ast.c $(FLAGS)

test_lexer:
	clang -o .bin/test_lexer lexer/lexer.c lexer/lexer_test.c token/token.c test/test.c $(FLAGS)

test_ast:
	clang -o .bin/test_ast ast/ast_test.c ast/ast.c token/token.c test/test.c $(FLAGS)

test_all:
	make test_lexer
	make test_parser
	./.bin/test_lexer && ./.bin/test_parser && ./.bin/test_ast

clean:
	rm -f .bin/monkey .bin/test_*
