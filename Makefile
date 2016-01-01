CFLAGS = -std=c11 -Isrc/ -Itests/unit/

ifdef DEBUG
	CFLAGS += -g
endif

CORE_OBJECTS = src/compiler/ast.o src/compiler/lexer.o src/compiler/parse_helpers.o
CORE_OBJECTS += src/compiler/token.o

CORE_OBJECTS += $(patsubst %.c,%.o,$(wildcard src/ds/*.c))
CORE_OBJECTS += $(patsubst %.c,%.o,$(wildcard src/model/*.c))
CORE_OBJECTS += $(patsubst %.c,%.o,$(wildcard src/runtime/*.c))
CORE_OBJECTS += $(patsubst %.c,%.o,$(wildcard src/debug/*.c))

EXEC_OBJECTS = src/entry.o

TEST_OBJECTS = tests/unit/suite.o

TEST_OBJECTS += $(patsubst %.c,%.o,$(wildcard tests/unit/ds/*.c))
TEST_OBJECTS += $(patsubst %.c,%.o,$(wildcard tests/unit/model/*.c))
TEST_OBJECTS += $(patsubst %.c,%.o,$(wildcard tests/unit/runtime/*.c))

.PHONY: test
test: bin/segment tests/units
	./tests/all.sh

bin/segment: src/compiler/grammar.c ${CORE_OBJECTS} ${EXEC_OBJECTS}
	mkdir -p bin/
	${CC} ${CORE_OBJECTS} ${EXEC_OBJECTS} -o bin/segment

src/compiler/lexer.c: src/compiler/lexer.rl src/compiler/grammar.c
	ragel -C -G2 src/compiler/lexer.rl

src/compiler/grammar.c: src/compiler/grammar.y
	cd src/compiler && lemon -s grammar.y

tests/units: ${CORE_OBJECTS} ${TEST_OBJECTS}
	${CC} ${CORE_OBJECTS} ${TEST_OBJECTS} -lcunit -o tests/suite

.PHONY: clean
clean:
	rm -f src/*.o
	rm -f src/compiler/grammar.c src/compiler/grammar.h src/compiler/grammar.out src/compiler/lexer.c
	rm -f src/debug/*.o src/ds/*.o src/model/*.o src/runtime/*.o src/compiler/*.o
	rm -f tests/unit/*.o tests/unit/ds/*.o tests/unit/model/*.o tests/unit/runtime/*.o
