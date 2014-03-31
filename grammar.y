// Grammer definition for zzz.

program ::= statementlist.

statementlist ::= .
statementlist ::= expr NEWLINE statementlist.
statementlist ::= expr SEMI statementlist.

expr ::= LPAREN expr RPAREN.
expr ::= var.
expr ::= literal.
expr ::= block.
expr ::= assignment.
expr ::= invocation.

// Literals

literal ::= INTEGER.
literal ::= FLOAT.
literal ::= TRUE.
literal ::= FALSE.
literal ::= STRING.
literal ::= SYMBOL.

// Blocks

block ::= LCURLY parameters statementlist RCURLY.

parameters ::= .
parameters ::= LPAREN parameterlist RPAREN.

// Assignment

assignment ::= var ASSIGNMENT expr.

var ::= IVAR.
var ::= TVAR.

// Invocation

invocation ::= receiver IDENTIFIER LPAREN commaargs RPAREN.
invocation ::= receiver IDENTIFIER spaceargs.

receiver ::= .
receiver ::= IDENTIFIER PERIOD.

// Operators and operator precedence.

%left ANDLIKE.
%left ORLIKE.
%right ASSIGNMENT.
%left PLUSLIKE MINUSLIKE.
%left MULTLIKE DIVLIKE MODLIKE.
%right EXPLIKE NOTLIKE.

invocation ::= receiver operator expr.

operator ::= ANDLIKE.
operator ::= ORLIKE.
operator ::= PLUSLIKE.
operator ::= MINUSLIKE.
operator ::= MULTLIKE.
operator ::= DIVLIKE.
operator ::= MODLIKE.
operator ::= EXPLIKE.

invocation ::= NOTLIKE expr.

// Argument and method parameter lists.

parameterlist ::= .
parameterlist ::= LPAREN commaparams RPAREN.

commaparams ::= parameter.
commaparams ::= commaparams COMMA parameter.

parameter ::= IDENTIFIER.
parameter ::= IDENTIFIER ASSIGNMENT expr.

commaargs ::= .
commaargs ::= commaargs COMMA arg.

spaceargs ::= .
spaceargs ::= spaceargs arg.

arg ::= expr.
arg ::= KEYWORD expr.
