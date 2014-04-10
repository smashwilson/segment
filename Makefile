ifdef DEBUG
	EXTRA=-g
else
	EXTRA=
endif

CFLAGS=-std=c99 ${EXTRA}
OBJECTS=src/token.o src/ast.o src/ast_printer.o src/parse_helpers.o src/lexer.o src/segment.o

bin/segment: src/grammar.c ${OBJECTS}
	mkdir -p bin/
	gcc ${OBJECTS} -o bin/segment

src/lexer.c: src/lexer.rl src/grammar.c
	ragel -C -G2 src/lexer.rl

src/grammar.c: src/grammar.y
	cd src && lemon -s grammar.y

.PHONY: clean
clean:
	rm -f src/*.o src/grammar.c src/grammar.h src/grammar.out src/lexer.c

.PHONY: test
test: bin/segment
	./tests/all.sh
