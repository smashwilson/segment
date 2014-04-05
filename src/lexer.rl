#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

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

  nonws = ^whitespace;
  noncontrol = ^control;
  nonop = ^op;

  iboundary = noncontrol & nonop & nonws;
  imiddle = nonws;

  identifier = iboundary imiddle iboundary | iboundary;
  symbol = ':' identifier | ':' string;

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
    '|' => { EMPTY(BAR); };
    ',' => { EMPTY(COMMA); };

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
}%%
/* Syntax Highlighting */

%% write data nofinal;

seg_statementlist_node *seg_parse(char *content, off_t length, seg_options *opts)
{
  /* Variables used by Ragel. */
  int cs, act;
  char *ts, *te;
  char *p = content;
  const char *pe = content + length;
  const char *eof = pe;

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
