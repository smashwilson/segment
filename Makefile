segment: grammar.o lexer.o
	gcc grammar.o lexer.o -o segment

lexer.c: lexer.rl
	ragel -C -G2 lexer.rl

grammar.c: grammar.y
	lemon -s grammar.y

clean:
	rm -f *.o grammar.c grammar.h grammar.out lexer.c

grammar.o: grammar.c
	gcc grammar.c

lexer.o: lexer.c
	gcc lexer.c
