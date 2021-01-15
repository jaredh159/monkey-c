FLAGS = -Wall -O -W -pedantic -g

monkey:
	clang -o .bin/monkey monkey.c repl/repl.c token/token.c lexer/lexer.c $(FLAGS)

test_parser:
	clang -o .bin/test_parser parser/parser_test.c parser/parser.c parser/parselets.c test/test.c lexer/lexer.c token/token.c ast/ast.c utils/argv.c utils/list.c $(FLAGS)

test_lexer:
	clang -o .bin/test_lexer lexer/lexer.c lexer/lexer_test.c token/token.c test/test.c utils/argv.c $(FLAGS)

test_ast:
	clang -o .bin/test_ast ast/ast_test.c ast/ast.c token/token.c test/test.c utils/argv.c utils/list.c $(FLAGS)

test_all:
	make test_lexer
	make test_parser
	make test_ast
	./.bin/test_lexer && ./.bin/test_parser && ./.bin/test_ast

all:
	make monkey
	make test_lexer
	make test_parser
	make test_ast

clean:
	rm -rf .bin/monkey .bin/test_*
