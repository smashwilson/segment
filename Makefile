CFLAGS = -std=c99

ifdef DEBUG
	CFLAGS += -g
endif

CORE_OBJECTS = src/token.o src/ast.o src/parse_helpers.o src/lexer.o src/symboltable.o
CORE_OBJECTS += src/ds/hashtable.o src/ds/murmur.o
CORE_OBJECTS += src/debug/ast_printer.o

EXEC_OBJECTS = src/segment.o

TEST_OBJECTS = tests/unit/suite.o
TEST_OBJECTS += tests/unit/symboltable_tests.o
TEST_OBJECTS += tests/unit/ds/hashtable_tests.o

bin/segment: src/grammar.c ${CORE_OBJECTS} ${EXEC_OBJECTS}
	mkdir -p bin/
	${CC} ${CORE_OBJECTS} ${EXEC_OBJECTS} -o bin/segment

src/lexer.c: src/lexer.rl src/grammar.c
	ragel -C -G2 src/lexer.rl

src/grammar.c: src/grammar.y
	cd src && lemon -s grammar.y

tests/units: ${CORE_OBJECTS} ${TEST_OBJECTS}
	${CC} ${CORE_OBJECTS} ${TEST_OBJECTS} -lcunit -o tests/suite

.PHONY: clean
clean:
	rm -f src/*.o src/grammar.c src/grammar.h src/grammar.out src/lexer.c
	rm -f src/ds/*.o
	rm -f src/debug/*.o
	rm -f tests/unit/*.o

.PHONY: test
test: bin/segment tests/units
	./tests/all.sh
