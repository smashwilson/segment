CFLAGS = -std=c99

ifdef DEBUG
	CFLAGS += -g
endif

CORE_OBJECTS = src/token.o src/ast.o src/ast_printer.o src/parse_helpers.o src/lexer.o
CORE_OBJECTS += src/ds/hashtable.o

EXEC_OBJECTS = src/segment.o

TEST_OBJECTS = tests/unit/suite.o
TEST_OBJECTS += tests/unit/ds/hashtable_tests.o

src/lexer.c: src/lexer.rl src/grammar.c
	ragel -C -G2 src/lexer.rl

src/grammar.c: src/grammar.y
	cd src && lemon -s grammar.y

bin/segment: src/grammar.c ${CORE_OBJECTS} ${EXEC_OBJECTS}
	mkdir -p bin/
	${CC} ${CORE_OBJECTS} ${EXEC_OBJECTS} -o bin/segment

tests/units: ${CORE_OBJECTS} ${TEST_OBJECTS}
	${CC} ${CORE_OBJECTS} ${TEST_OBJECTS} -lcunit -o tests/suite

.PHONY: clean
clean:
	rm -f src/*.o src/grammar.c src/grammar.h src/grammar.out src/lexer.c

.PHONY: test
test: bin/segment tests/units
	./tests/all.sh
