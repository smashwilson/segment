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

// Paren method call, explicit receiver

invocation ::= receiver METHODNAME commaargs RPAREN.

// Paren method call, implicit receiver

invocation ::= METHODNAME commaargs RPAREN.

// Space method call, explicit receiver

spaceinvocation ::= receiver IDENTIFIER spaceargs.

// Space method call, implicit receiver

spaceinvocation ::= IDENTIFIER spaceargs.

receiver ::= expr PERIOD.

// Argument lists.

commaargs ::= .
commaargs ::= commaargs COMMA commaarg.

commaarg ::= statement.
commaarg ::= KEYWORD statement.

spaceargs ::= spacearg.
spaceargs ::= spaceargs spacearg.

spacearg ::= expr.
spacearg ::= KEYWORD expr.
