CFLAGS = -std=c99 -Isrc/ -Itests/unit/

ifdef DEBUG
	CFLAGS += -g
endif

CORE_OBJECTS = src/lexer.o
CORE_OBJECTS += src/token.o src/ast.o
CORE_OBJECTS += src/parse_helpers.o

CORE_OBJECTS += src/ds/stringtable.o src/ds/ptrtable.o src/ds/plugtable.o src/ds/murmur.o
CORE_OBJECTS += src/model/object.o
CORE_OBJECTS += src/runtime/symboltable.o src/runtime/runtime.o
CORE_OBJECTS += src/debug/ast_printer.o src/debug/symbol_printer.o

EXEC_OBJECTS = src/entry.o

TEST_OBJECTS = tests/unit/suite.o

TEST_OBJECTS += tests/unit/ds/stringtable_tests.o tests/unit/ds/ptrtable_tests.o
TEST_OBJECTS += tests/unit/ds/plugtable_tests.o

TEST_OBJECTS += tests/unit/model/object_tests.o

TEST_OBJECTS += tests/unit/runtime/symboltable_tests.o
TEST_OBJECTS += tests/unit/runtime/runtime_tests.o

.PHONY: test
test: bin/segment tests/units
	./tests/all.sh

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
