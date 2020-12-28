monkey:
	clang -o .bin/monkey monkey.c repl/repl.c token/token.c lexer/lexer.c

test_lexer:
	clang -o .bin/test_lexer lexer/lexer.c lexer/lexer_test.c token/token.c
	.bin/test_lexer

clean:
	rm -f .bin/monkey .bin/test_lexer
