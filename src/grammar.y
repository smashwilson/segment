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
%type block { seg_block_node* }
%type parameters { seg_parameter_list* }
%type commaparams { seg_parameter_list* }
%type parameter { seg_parameter_list* }

%extra_argument { seg_program_node *program_node }

// Grammar definition.

program (OUT) ::= statementlist (LIST).
{
  program_node->root = LIST;
  OUT = LIST;
}

statementlist (OUT) ::= statement (ONLY).
{
  OUT = malloc(sizeof(seg_statementlist_node));
  OUT->first = ONLY;
  OUT->last = ONLY;
}

statementlist (OUT) ::= statementlist (LIST) NEWLINE statement (NEW).
{
  if (NEW != NULL) {
    if (LIST->last != NULL) {
      LIST->last->next = NEW;
    } else {
      LIST->first = NEW;
    }
    LIST->last = NEW;
  }
  OUT = LIST;
}

statementlist (OUT) ::= statementlist (LIST) SEMI statement (NEW).
{
  if (NEW != NULL) {
    if (LIST->last != NULL) {
      LIST->last->next = NEW;
    } else {
      LIST->first = NEW;
    }
    LIST->last = NEW;
  }
  OUT = LIST;
}

statement (OUT) ::= . { OUT = NULL; }

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
  OUT->child_kind = SEG_INTEGER;
  OUT->child.integer = inode;
}

expr ::= FLOAT.
expr ::= TRUE.
expr ::= FALSE.
expr ::= STRING.
expr ::= SYMBOL.

// Compound Expressions

expr ::= LPAREN statement RPAREN.
expr ::= IDENTIFIER.
expr (OUT) ::= block (B).
{
  OUT = malloc(sizeof(seg_expr_node));
  OUT->child_kind = SEG_BLOCK;
  OUT->child.block = B;
}

expr ::= assignment.
expr (OUT) ::= invocation (I). { OUT = I; }

// Blocks

block (OUT) ::= LCURLY parameters (PARAMS) statementlist (BODY) RCURLY.
{
  OUT = malloc(sizeof(seg_block_node));
  OUT->parameters = PARAMS;
  OUT->body = BODY;
}

parameters ::= .
parameters (OUT) ::= BAR commaparams (IN) BAR. { OUT = IN; }

commaparams (OUT) ::= parameter (IN). { OUT = IN; }
commaparams (OUT) ::= commaparams (LIST) COMMA parameter (NEW).
{
  /* Parameters are pushed in reverse order. */
  LIST->next = NEW;
  OUT = NEW;
}

parameter (OUT) ::= IDENTIFIER (ID).
{
  OUT = malloc(sizeof(seg_parameter_list));
  OUT->name = seg_token_as_string(ID);
  OUT->next = NULL;
  seg_delete_token(ID);
}

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
  seg_delete_token(AND);
  op->left = LHS;
  op->right = RHS;

  OUT = malloc(sizeof(seg_expr_node));
  OUT->child_kind = SEG_BINOP;
  OUT->child.binop = op;
}

invocation ::= expr ORLIKE expr.

invocation (OUT) ::= expr (LHS) PLUSLIKE (PLUS) expr (RHS).
{
  seg_binop_node *op = malloc(sizeof(seg_binop_node));
  op->selector = seg_token_as_string(PLUS);
  seg_delete_token(PLUS);
  op->left = LHS;
  op->right = RHS;

  OUT = malloc(sizeof(seg_expr_node));
  OUT->child_kind = SEG_BINOP;
  OUT->child.binop = op;
}

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
