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

static void report(const char *name, const char *ts, const char *te, seg_options *opts) {
  if (! opts->lexer_debug) {
    return ;
  }
  int length = (int) (te - ts);
  if (*ts == '\n' && length == 1) {
    printf("tok: %s [\\n]\n", name);
  } else {
    printf("tok: %s [%.*s]\n", name, length, ts);
  }
}

#define EMPTY(CODE) \
  do { \
    report(#CODE, ts, te, opts); \
    Parse(parser, CODE, NULL, &program); \
  } while (0)

#define CAPTURE(CODE) \
  do { \
    report(#CODE, ts, te, opts); \
    Parse(parser, CODE, seg_new_token(ts, te), &program); \
  } while (0)

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

  control = [(){};=.|,];
  sigil = [%@];
  op = [&|+\-*/%^];

  nonws = ^whitespace & [^\r\n];
  noncontrol = ^control;
  nonsigil = ^sigil;
  nonop = ^op;
  nonnumeric = [^0-9];

  imiddle = nonws & noncontrol;
  istart = imiddle & nonsigil & nonnumeric;
  iend = imiddle;
  ionly = istart & iend;

  identifier = istart imiddle* iend | ionly;
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
    ';' => { EMPTY(SEMI); };
    '\n' => { EMPTY(NEWLINE); };
    '=' => { EMPTY(ASSIGNMENT); };
    '.' => { EMPTY(PERIOD); };
    ',' => { EMPTY(COMMA); };

    # Eat whitespace and newlines at the start of a block.
    # Otherwise, the extraneous NEWLINE tokens will confuse the parser.
    '{' (whitespace | [\r\n])* => { EMPTY(BLOCKSTART); };
    '}' => { EMPTY(BLOCKEND); };

    '|' => {
      EMPTY(BAR);
      fcall blockargs;
    };

    identifier '(' => { CAPTURE(METHODNAME); };

    identifier ':' => { CAPTURE(KEYWORD); };

    identifier? '&' => { CAPTURE(ANDLIKE); };
    identifier? '|' => { CAPTURE(ORLIKE); };
    identifier? '+' => { CAPTURE(PLUSLIKE); };
    identifier? '-' => { CAPTURE(MINUSLIKE); };
    identifier? '*' => { CAPTURE(MULTLIKE); };
    identifier? '/' => { CAPTURE(DIVLIKE); };
    identifier? '%' => { CAPTURE(MODLIKE); };
    identifier? '^' => { CAPTURE(EXPLIKE); };
    '!' => { EMPTY(NOTLIKE); };

    identifier => { CAPTURE(IDENTIFIER); };
    '@' identifier => { CAPTURE(IVAR); };
    '%' identifier => { CAPTURE(TVAR); };

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
