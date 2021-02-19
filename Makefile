FLAGS = -Wall -O -W -pedantic -g

monkey:
	clang -o .bin/monkey monkey.c repl/repl.c token/token.c lexer/lexer.c parser/parser.c parser/parselets.c evaluator/evaluator.c evaluator/builtins.c object/object.c object/environment.c utils/argv.c ast/ast.c utils/list.c $(FLAGS)

test_parser:
	clang -o .bin/test_parser parser/parser_test.c parser/parser.c parser/parselets.c test/test.c lexer/lexer.c token/token.c ast/ast.c utils/argv.c utils/list.c $(FLAGS)

test_lexer:
	clang -o .bin/test_lexer lexer/lexer.c lexer/lexer_test.c token/token.c test/test.c utils/argv.c $(FLAGS)

test_object:
	clang -o .bin/test_object object/object_test.c object/object.c token/token.c test/test.c utils/argv.c utils/list.c ast/ast.c $(FLAGS)

test_ast:
	clang -o .bin/test_ast ast/ast_test.c ast/ast.c token/token.c test/test.c utils/argv.c utils/list.c $(FLAGS)

test_eval:
	clang -o .bin/test_eval evaluator/evaluator_test.c evaluator/evaluator.c evaluator/builtins.c object/object.c object/environment.c parser/parser.c lexer/lexer.c parser/parselets.c ast/ast.c token/token.c test/test.c utils/argv.c utils/list.c $(FLAGS)

test_all:
	make test_lexer
	make test_parser
	make test_ast
	make test_eval
	printf "\nLEXER:  " && ./.bin/test_lexer && printf "AST:    " && ./.bin/test_ast && printf "EVAL:   " && ./.bin/test_eval && printf "PARSER: " && ./.bin/test_parser && echo

all:
	make monkey
	make test_lexer
	make test_parser
	make test_ast

clean:
	rm -rf .bin/monkey .bin/test_*
