#include <stdlib.h>

#include "grammar.h"
#include "grammar.c"

#define EMIT(CODE) Parse(parser, CODE, 0)

%%{
  machine segment_lexer;

  comment = '#' [^\n]* '\n';
  whitespace = [ \t]+;

  integer = ('+'|'-')?[0-9]+;
  float = ('+'|'-')?[0-9]+'.'[0-9]+;
  true = 'true';
  false = 'false';
  string = '"' [^"]* '"';

  identifier = [^ \t\r\n({.;"']+;
  symbol = ':' identifier | ':' string;

  main := |*
    comment;

    integer => { EMIT(INTEGER); };
    float => { EMIT(FLOAT); };
    true => { EMIT(TRUE); };
    false => { EMIT(FALSE); };
    string => { EMIT(STRING); };
    symbol => { EMIT(SYMBOL); };

    '(' => { EMIT(LPAREN); };
    ')' => { EMIT(RPAREN); };
    '{' => { EMIT(LCURLY); };
    '}' => { EMIT(RCURLY); };
    ';' => { EMIT(SEMI); };
    '\n' => { EMIT(NEWLINE); };
    '=' => { EMIT(ASSIGNMENT); };
    '.' => { EMIT(PERIOD); };
    '|' => { EMIT(BAR); };
    ',' => { EMIT(COMMA); };

    identifier => { EMIT(IDENTIFIER); };
    '@' identifier => { EMIT(IVAR); };
    '%' identifier => { EMIT(TVAR); };

    identifier '(' => { EMIT(METHODNAME); };

    identifier ':' => { EMIT(KEYWORD); };

    identifier? '&' => { EMIT(ANDLIKE); };
    identifier? '|' => { EMIT(ORLIKE); };
    identifier? '+' => { EMIT(PLUSLIKE); };
    identifier? '-' => { EMIT(MINUSLIKE); };
    identifier? '*' => { EMIT(MULTLIKE); };
    identifier? '/' => { EMIT(DIVLIKE); };
    identifier? '%' => { EMIT(MODLIKE); };
    identifier? '^' => { EMIT(EXPLIKE); };
    '!' => { EMIT(NOTLIKE); };

    whitespace;
  *|;
}%%
/* Syntax Highlighting */

%% write data nofinal;

int compile(char *content, size_t length)
{
  /* Variables used by Ragel. */
  int cs, act;
  char *ts, *te;
  char *p = content;
  const char *pe = content + length;
  const char *eof = pe;

  void *parser = ParseAlloc(malloc);

  %% write init;

  %% write exec;

  Parse(parser, 0, NULL);
  ParseFree(parser, free);

  return 0;
}
