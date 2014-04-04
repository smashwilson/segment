%include {
  #include <assert.h>
  #include <stdlib.h>
  #include <stdio.h>

  #include "ast.h"
  #include "token.h"
}

// Grammar definition for segment.

// Operators and operator precedence.

%left PERIOD IDENTIFIER.
%left ANDLIKE.
%left ORLIKE.
%right ASSIGNMENT.
%left PLUSLIKE MINUSLIKE.
%left MULTLIKE DIVLIKE MODLIKE.
%right EXPLIKE NOTLIKE.

// Nonterminal and token types.

%token_type { seg_token* }

%type program { seg_statementlist_node* }
%type statementlist { seg_statementlist_node* }
%type statement { seg_statement_node* }
%type expr { seg_expr_node* }
%type invocation { seg_expr_node* }

%extra_argument { seg_program_node *program_node }

// Grammar definition.

program (OUT) ::= statementlist (LIST).
{
  program_node->root = LIST;
  OUT = LIST;
}

statementlist (OUT) ::= .
{
  OUT = malloc(sizeof(seg_statementlist_node));
  OUT->first = NULL;
}

statementlist (OUT) ::= statement (NEW) NEWLINE statementlist (LIST).
{
  NEW->next = LIST->first;
  LIST->first = NEW;
  OUT = LIST;
}

statementlist (OUT) ::= statement (NEW) SEMI statementlist (LIST).
{
  NEW->next = LIST->first;
  LIST->first = NEW;
  OUT = LIST;
}

statement (OUT) ::= expr (E).
{
  OUT = malloc(sizeof(seg_statement_node));
  OUT->child_kind = SEG_EXPR;
  OUT->child.expr = E;
}

statement ::= spaceinvocation.

// Literals

expr (OUT) ::= INTEGER (L).
{
  seg_integer_node *inode = malloc(sizeof(seg_integer_node));
  inode->value = seg_token_as_integer(L);
  seg_delete_token(L);

  OUT = malloc(sizeof(seg_expr_node));
  OUT->kind = SEG_INTEGER;
  OUT->expr.integer = inode;
}

expr ::= FLOAT.
expr ::= TRUE.
expr ::= FALSE.
expr ::= STRING.
expr ::= SYMBOL.

// Compound Expressions

expr ::= LPAREN statement RPAREN.
expr ::= IDENTIFIER.
expr ::= block.
expr ::= assignment.
expr (OUT) ::= invocation (I). { OUT = I; }

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

invocation (OUT) ::= expr (LHS) ANDLIKE (AND) expr (RHS).
{
  seg_binop_node *op = malloc(sizeof(seg_binop_node));
  op->selector = seg_token_as_string(AND);
  op->left = LHS;
  op->right = RHS;

  OUT = malloc(sizeof(seg_expr_node));
  OUT->expr.binop = op;
  OUT->kind = SEG_BINOP;
}

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
