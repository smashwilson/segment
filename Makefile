bin/segment: src/grammar.o src/lexer.o
	mkdir -p bin/
	gcc src/grammar.o src/lexer.o -o bin/segment

src/lexer.c: src/lexer.rl
	ragel -C -G2 src/lexer.rl

src/grammar.c: src/grammar.y
	cd src
	lemon -s grammar.y

clean:
	rm -f src/*.o src/grammar.c src/grammar.h src/grammar.out src/lexer.c

src/grammar.o: src/grammar.c
	gcc grammar.c

src/lexer.o: src/lexer.c
	gcc lexer.c
