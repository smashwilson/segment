#include "grammar.h"
#include "grammar.c"

%%{

  machine zzz_lexer;

  true = 'true';
  false = 'false';

  integer = ('+'|'-')?[0-9]+;
  float = ('+'|'-')?[0-9]+'.'[0-9]+;
  string = '"' [^"]* '"';

  identifier = [^ \t\r\n({.;"']+;

  main := |*

    '#' [^\n]* '\n';

    integer => { /* INTEGER */ };
    float => { /* FLOAT */ };
    true => { /* TRUE */ };
    false => { /* FALSE */ };
    string => { /* STRING */ };
    ':' identifier | ':' string => {
      /* SYMBOL */
    };

    '(' => { /* LPAREN */ };
    ')' => { /* RPAREN */ };
    '{' => { /* LCURLY */ };
    '}' => { /* RCURLY */ };
    ';' => { /* SEMI */ };
    '\n' => { /* NEWLINE */ };
    '=' => { /* ASSIGNMENT */ };
    '.' => { /* PERIOD */ };

    identifier => { /* IDENTIFIER */ };
    '@' identifier => { /* IVAR */ };
    '%' identifier => { /* TVAR */ };

    identifier ':' => { /* KEYWORD */ };

    identifier? '&' => { /* ANDLIKE */ };
    identifier? '|' => { /* ORLIKE */ };
    identifier? '*' => { /* MULTLIKE */ };
    identifier? '/' => { /* DIVLIKE */ };
    identifier? '%' => { /* MODLIKE */ };
    '!' => { /* NOTLIKE */ };

    space;
  *|

}%%
/* Syntax Highlighting */

%% write data nofinal;

int compile()
{
  int cs, act;
  const char *ts = 0, *te = 0;
  void *parser = ParseAlloc(malloc);

  %% write init;

  %% write exec;

  if (cs == lexer_error) {
    state->errors++;
    error(state, "invalid character '%c'\n", p[0]);
  } else {
    Parse(parser, 0, NULL);
  }

  ParseFree(parser, free);

  return state->errors > 0 ? 0 : 1;
}
