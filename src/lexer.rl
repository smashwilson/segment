#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "lexer.h"
#include "ast.h"
#include "token.h"
#include "segment.h"

#include "grammar.h"
#include "grammar.c"

static void report(const char *name, const char *ts, const char *te) {
  int length = (int) (te - ts);
  printf("tok: %s [%.*s]\n", name, length, ts);
}

#define EMPTY(CODE) \
  if (opts->lexer_debug) { \
    report(#CODE, ts, te);\
  };\
  Parse(parser, CODE, NULL, &program)

#define CAPTURE(CODE) \
  if (opts->lexer_debug) { \
    report(#CODE, ts, te);\
  };\
  Parse(parser, CODE, seg_new_token(ts, te), &program)

%%{
  machine segment_lexer;

  comment = '#' [^\n]* '\n';
  whitespace = [ \t]+;

  integer = ('+'|'-')?[0-9]+;
  float = ('+'|'-')?[0-9]+'.'[0-9]+;
  true = 'true';
  false = 'false';
  string = '"' [^"]* '"';
  # Syntax highlight fix. '

  control = [(){};=.|,%@];
  op = [&|+\-*/%^];

  nonws = ^whitespace & [^\r\n];
  noncontrol = ^control;
  nonop = ^op;
  nonnumeric = [^0-9];

  iboundary = nonws & noncontrol & nonnumeric;
  imiddle = nonws;

  identifier = iboundary imiddle* iboundary | iboundary;
  symbol = ':' identifier | ':' string;

  blockargs := |*
    comment;
    whitespace;
    [\r\n];

    identifier => { CAPTURE(IDENTIFIER); };
    ',' => { EMPTY(COMMA); };
    '|' => {
      EMPTY(BAR);
      fret;
    };
  *|;

  main := |*
    comment;

    integer => { CAPTURE(INTEGER); };
    float => { EMPTY(FLOAT); };
    true => { EMPTY(TRUE); };
    false => { EMPTY(FALSE); };
    string => { EMPTY(STRING); };
    symbol => { EMPTY(SYMBOL); };

    '(' => { EMPTY(LPAREN); };
    ')' => { EMPTY(RPAREN); };
    '{' => { EMPTY(LCURLY); };
    '}' => { EMPTY(RCURLY); };
    ';' => { EMPTY(SEMI); };
    '\n' => { EMPTY(NEWLINE); };
    '=' => { EMPTY(ASSIGNMENT); };
    '.' => { EMPTY(PERIOD); };
    ',' => { EMPTY(COMMA); };

    '|' => {
      EMPTY(BAR);
      fcall blockargs;
    };

    identifier '(' => { EMPTY(METHODNAME); };

    identifier ':' => { EMPTY(KEYWORD); };

    identifier? '&' => { CAPTURE(ANDLIKE); };
    identifier? '|' => { EMPTY(ORLIKE); };
    identifier? '+' => { CAPTURE(PLUSLIKE); };
    identifier? '-' => { EMPTY(MINUSLIKE); };
    identifier? '*' => { EMPTY(MULTLIKE); };
    identifier? '/' => { EMPTY(DIVLIKE); };
    identifier? '%' => { EMPTY(MODLIKE); };
    identifier? '^' => { EMPTY(EXPLIKE); };
    '!' => { EMPTY(NOTLIKE); };

    identifier => { CAPTURE(IDENTIFIER); };
    '@' identifier => { EMPTY(IVAR); };
    '%' identifier => { EMPTY(TVAR); };

    whitespace;
  *|;

  # Stack management

  prepush {
    if (top >= stack_size) {
      if (opts->verbose) {
        printf("Growing stack from %d to %d.\n", stack_size, stack_size + RAGEL_STACK_INCR);
      }
      stack = realloc(stack, stack_size + RAGEL_STACK_INCR);
    }
  }

  postpop {
    if (stack_size - top >= RAGEL_STACK_INCR) {
      if (opts->verbose) {
        printf("Shrinking stack from %d to %d.\n", stack_size, stack_size - RAGEL_STACK_INCR);
      }
      stack = realloc(stack, stack_size - RAGEL_STACK_INCR);
    }
  }
}%%
/* Syntax Highlighting */

%% write data nofinal;

seg_statementlist_node *seg_parse(char *content, off_t length, seg_options *opts)
{
  /* Variables used by Ragel. */
  int cs, act, top;
  char *ts, *te;
  char *p = content;
  const char *pe = content + length;
  const char *eof = pe;

  int *stack = malloc(sizeof(int) * RAGEL_INIT_STACK_SIZE);
  int stack_size = RAGEL_INIT_STACK_SIZE;

  /* Parser state */
  int lexer_error = 0;
  void *parser = ParseAlloc(malloc);
  seg_program_node program;
  program.root = NULL;

  if (opts->verbose) {
    puts("Starting lexer.");
  }

  %% write init;

  %% write exec;

  if (cs == segment_lexer_error) {
    lexer_error = 1;
  } else {
    Parse(parser, 0, NULL, &program);
  }

  if (opts->verbose) {
    puts("Lexing complete.");
  }

  ParseFree(parser, free);

  return program.root;
}
