zzz: grammar.o lexer.o
	gcc grammar.o lexer.o -o zzz

lexer.c: lexer.rl
	ragel -C -G2 lexer.rl

grammar.c: grammar.y
	lemon -s grammar.y

clean:
	rm -f *.o grammar.c grammar.h lexer.c

grammar.o: grammar.c
	gcc grammar.c

lexer.o: lexer.c
	gcc lexer.c
