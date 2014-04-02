%include { #include<assert.h> }

// Grammar definition for segment.

program ::= statementlist.

statementlist ::= .
statementlist ::= expr NEWLINE statementlist.
statementlist ::= expr SEMI statementlist.

expr ::= LPAREN expr RPAREN.
expr ::= var.
expr ::= literal.
expr ::= block.
expr ::= assignment.
expr ::= invocationchain.
expr ::= operatorchain.

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

invocationchain ::= invocation.
invocationchain ::= invocation PERIOD invocationchain.

invocation ::= IDENTIFIER args.

args ::= LPAREN commaargs RPAREN.
args ::= spaceargs.

// Operators and operator precedence.

%left ANDLIKE.
%left ORLIKE.
%right ASSIGNMENT.
%left PLUSLIKE MINUSLIKE.
%left MULTLIKE DIVLIKE MODLIKE.
%right EXPLIKE NOTLIKE.
%left PERIOD.

operatorchain ::= expr operator operatorchain.

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
commaargs ::= commaargs WS COMMA WS arg.

spaceargs ::= .
spaceargs ::= spaceargs WS arg.

arg ::= expr.
arg ::= KEYWORD expr.
