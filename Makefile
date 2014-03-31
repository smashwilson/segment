lexer.c: lexer.rl
	ragel -C -G2 lexer.rl

grammar.c: grammar.y
	lemon -s grammar.y

clean:
	rm -f *.o grammar.c grammar.h lexer.c

zzz: grammar.c lexer.c
