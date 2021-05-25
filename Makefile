FLAGS = -Wall -O -W -pedantic -g

.SILENT: test_all test_lexer test_parser test_ast test_code test_compiler test_vm test_eval test_bb

monkey:
	clang -o .bin/monkey monkey.c repl/repl.c token/token.c code/code.c vm/vm.c compiler/compiler.c lexer/lexer.c parser/parser.c parser/parselets.c evaluator/evaluator.c evaluator/builtins.c object/object.c object/environment.c utils/argv.c ast/ast.c utils/list.c $(FLAGS)

test_parser:
	clang -o .bin/test_parser parser/parser_test.c parser/parser.c parser/parselets.c test/test.c lexer/lexer.c token/token.c object/object.c ast/ast.c utils/argv.c utils/list.c $(FLAGS)

test_lexer:
	clang -o .bin/test_lexer lexer/lexer.c lexer/lexer_test.c token/token.c object/object.c utils/list.c ast/ast.c test/test.c utils/argv.c $(FLAGS)

test_object:
	clang -o .bin/test_object object/object_test.c object/object.c token/token.c test/test.c utils/argv.c utils/list.c ast/ast.c $(FLAGS)

test_ast:
	clang -o .bin/test_ast ast/ast_test.c ast/ast.c token/token.c test/test.c object/object.c utils/argv.c utils/list.c $(FLAGS)

test_eval:
	clang -o .bin/test_eval evaluator/evaluator_test.c evaluator/evaluator.c evaluator/builtins.c object/object.c object/environment.c parser/parser.c lexer/lexer.c parser/parselets.c ast/ast.c token/token.c test/test.c utils/argv.c utils/list.c $(FLAGS)

test_compiler:
	clang -o .bin/test_compiler compiler/compiler.c compiler/compiler_test.c code/code.c parser/parser.c parser/parselets.c object/object.c lexer/lexer.c utils/list.c ast/ast.c test/test.c token/token.c utils/argv.c $(FLAGS)

test_code:
	clang -o .bin/test_code code/code.c code/code_test.c test/test.c token/token.c utils/list.c ast/ast.c object/object.c utils/argv.c $(FLAGS)

test_vm:
	clang -o .bin/test_vm vm/vm.c vm/vm_test.c compiler/compiler.c test/test.c object/object.c code/code.c ast/ast.c token/token.c parser/parser.c parser/parselets.c lexer/lexer.c utils/list.c utils/argv.c $(FLAGS)

FMT = "%-10s"

test_all:
	make test_lexer
	make test_parser
	make test_ast
	make test_eval
	make test_code
	make test_compiler
	make test_vm
	echo
	printf $(FMT) "LEXER:"
	TEST_ALL=true ./.bin/test_lexer
	printf $(FMT) "AST:"
	TEST_ALL=true ./.bin/test_ast
	printf $(FMT) "EVAL:"
	TEST_ALL=true ./.bin/test_eval
	printf $(FMT) "PARSER:"
	TEST_ALL=true ./.bin/test_parser
	printf $(FMT) "CODE:"
	TEST_ALL=true ./.bin/test_code
	printf $(FMT) "COMPILER:"
	TEST_ALL=true ./.bin/test_compiler
	printf $(FMT) "VM:"
	TEST_ALL=true ./.bin/test_vm
	echo

# bb = "book 2"
test_bb:
	make test_code
	make test_compiler
	make test_vm
	echo
	printf $(FMT) "CODE:"
	TEST_ALL=true ./.bin/test_code
	printf $(FMT) "COMPILER:"
	TEST_ALL=true ./.bin/test_compiler
	printf $(FMT) "VM:"
	TEST_ALL=true ./.bin/test_vm
	echo

all:
	make monkey
	make test_lexer
	make test_parser
	make test_ast
	make test_code

clean:
	rm -rf .bin/monkey .bin/test_*
