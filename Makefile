PKGCONFIG_LIBS = icu-uc

CFLAGS = -std=c99
ifdef DEBUG
	CFLAGS += -g
endif
CFLAGS_PKGCONFIG = $(shell pkg-config --cflags ${PKGCONFIG_LIBS})
CFLAGS += ${CFLAGS_PKGCONFIG}

LDFLAGS_PKGCONFIG = $(shell pkg-config --libs ${PKGCONFIG_LIBS})
LDFLAGS += ${LDFLAGS_PKGCONFIG}

OBJECTS = src/token.o src/ast.o src/ast_printer.o src/lexer.o src/segment.o

bin/segment: src/grammar.c ${OBJECTS}
	mkdir -p bin/
	${CC} ${LDFLAGS} ${OBJECTS} -o bin/segment

src/lexer.c: src/lexer.rl src/grammar.c
	ragel -C -G2 src/lexer.rl

src/grammar.c: src/grammar.y
	cd src && lemon -s grammar.y

clean:
	rm -f src/*.o src/grammar.c src/grammar.h src/grammar.out src/lexer.c
