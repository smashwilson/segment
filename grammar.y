%include { #include <assert.h> }

// Grammar definition for segment.

// Operators and operator precedence.

%left PERIOD IDENTIFIER.
%left ANDLIKE.
%left ORLIKE.
%right ASSIGNMENT.
%left PLUSLIKE MINUSLIKE.
%left MULTLIKE DIVLIKE MODLIKE.
%right EXPLIKE NOTLIKE.

// Grammar definition.

program ::= statementlist.

statementlist ::= .
statementlist ::= statement NEWLINE statementlist.
statementlist ::= statement SEMI statementlist.

statement ::= expr.
statement ::= spaceinvocation.

expr ::= LPAREN statement RPAREN.
expr ::= literal.
expr ::= IDENTIFIER.
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
parameters ::= BAR commaparams BAR.

commaparams ::= parameter.
commaparams ::= commaparams COMMA parameter.

parameter ::= IDENTIFIER.
parameter ::= IDENTIFIER ASSIGNMENT expr.

// Assignment

assignment ::= lhs ASSIGNMENT expr.

lhs ::= IVAR.
lhs ::= TVAR.

// Invocation

// Binary operators

invocation ::= expr ANDLIKE expr.
invocation ::= expr ORLIKE expr.
invocation ::= expr PLUSLIKE expr.
invocation ::= expr MINUSLIKE expr.
invocation ::= expr MULTLIKE expr.
invocation ::= expr DIVLIKE expr.
invocation ::= expr MODLIKE expr.
invocation ::= expr EXPLIKE expr.

// Unary operators

invocation ::= NOTLIKE expr.

// Method call, explicit receiver

invocation ::= receiver METHODNAME commaargs RPAREN.

// Method call, space invocation.

spaceinvocation ::= receiver IDENTIFIER spaceargs.

receiver ::= expr PERIOD.

// invocation ::= METHODNAME commaargs RPAREN.
// invocation ::= IDENTIFIER spaceargs.

// Argument lists.

commaargs ::= .
commaargs ::= commaargs COMMA arg.

spaceargs ::= arg.
spaceargs ::= spaceargs arg.

arg ::= expr.
arg ::= KEYWORD expr.
