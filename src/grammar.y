%include {
  #include <assert.h>
  #include <stdlib.h>
  #include <stdio.h>

  #include "ast.h"
  #include "token.h"
  #include "parse_helpers.h"
}

// Grammar definition for segment.

// Operators and operator precedence.

%left PERIOD.
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
%type maybestatement { seg_expr_node* }
%type statement { seg_expr_node* }
%type expr { seg_expr_node* }
%type invocation { seg_expr_node* }
%type spaceinvocation { seg_expr_node* }
%type blockstart { seg_block_node* }
%type block { seg_block_node* }
%type parameters { seg_parameter_list* }
%type commaparams { seg_parameter_list* }
%type parameter { seg_parameter_list* }

%type commaargs { seg_arg_list* }
%type commaarg  { seg_arg_list* }
%type spaceargs { seg_arg_list* }
%type spacearg  { seg_arg_list* }

%extra_argument { seg_parser_state *parser_state }

// Grammar definition.

program (OUT) ::= statementlist (LIST).
{
  parser_state->root = LIST;
  OUT = LIST;
}

statementlist (OUT) ::= maybestatement (ONLY).
{
  OUT = malloc(sizeof(seg_statementlist_node));
  OUT->first = ONLY;
  OUT->last = ONLY;
}

statementlist (OUT) ::= statementlist (LIST) NEWLINE maybestatement (MAYBE).
{
  OUT = seg_append_statement(LIST, MAYBE);
}

statementlist (OUT) ::= statementlist (LIST) SEMI maybestatement (MAYBE).
{
  OUT = seg_append_statement(LIST, MAYBE);
}

maybestatement ::= .
maybestatement (OUT) ::= statement (IN). { OUT = IN; }

statement (OUT) ::= expr (IN). { OUT = IN; }
statement (OUT) ::= spaceinvocation (IN). { OUT = IN; }

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

expr (OUT) ::= LPAREN statement (IN) RPAREN. { OUT = IN; }
expr (OUT) ::= IDENTIFIER (V).
{
  size_t length;
  const char *name = seg_token_as_string(V, &length);
  seg_delete_token(V);

  OUT = malloc(sizeof(seg_expr_node));

  if (seg_parser_isarg(parser_state, name, length)) {
    seg_var_node *varnode = malloc(sizeof(seg_var_node));
    varnode->varname = name;
    varnode->length = length;

    OUT->child_kind = SEG_VAR;
    OUT->child.var = varnode;
  } else {
    seg_methodcall_node *methodcall = malloc(sizeof(seg_methodcall_node));
    methodcall->selector = name;
    methodcall->length = length;
    methodcall->receiver = seg_implicit_self();
    methodcall->args = NULL;

    OUT->child_kind = SEG_METHODCALL;
    OUT->child.methodcall = methodcall;
  }
}

expr (OUT) ::= block (B).
{
  OUT = malloc(sizeof(seg_expr_node));
  OUT->child_kind = SEG_BLOCK;
  OUT->child.block = B;
}

expr ::= assignment.
expr (OUT) ::= invocation (I). { OUT = I; }

// Blocks

block (OUT) ::= blockstart (BLK) parameters statementlist (BODY) BLOCKEND.
{
  OUT = BLK;
  OUT->body = BODY;

  /*
    Parameters are pushed in reverse order.
    This restores them to the correct order.
  */
  seg_parameter_list *params = seg_reverse_params(OUT->parameters);
  OUT->parameters = params;

  seg_parser_popcontext(parser_state);
}

blockstart (OUT) ::= BLOCKSTART.
{
  OUT = malloc(sizeof(seg_block_node));
  OUT->parameters = NULL;
  OUT->body = NULL;

  seg_parser_pushcontext(parser_state, OUT);
}

parameters ::= .
parameters ::= BAR commaparams BAR.

commaparams ::= parameter (IN).
{
  seg_parser_addparam(parser_state, IN);
}

commaparams ::= commaparams COMMA parameter (NEW).
{
  seg_parser_addparam(parser_state, NEW);
}

parameter (OUT) ::= IDENTIFIER (ID).
{
  OUT = malloc(sizeof(seg_parameter_list));
  OUT->name = seg_token_as_string(ID, &(OUT->length));
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

invocation (OUT) ::= expr (LHS) ANDLIKE (OP) expr (RHS). { OUT = seg_parse_binop(LHS, OP, RHS); }
invocation (OUT) ::= expr (LHS) ORLIKE (OP) expr (RHS). { OUT = seg_parse_binop(LHS, OP, RHS); }
invocation (OUT) ::= expr (LHS) PLUSLIKE (OP) expr (RHS). { OUT = seg_parse_binop(LHS, OP, RHS); }
invocation (OUT) ::= expr (LHS) MINUSLIKE (OP) expr (RHS). { OUT = seg_parse_binop(LHS, OP, RHS); }
invocation (OUT) ::= expr (LHS) MULTLIKE (OP) expr (RHS). { OUT = seg_parse_binop(LHS, OP, RHS); }
invocation (OUT) ::= expr (LHS) DIVLIKE (OP) expr (RHS). { OUT = seg_parse_binop(LHS, OP, RHS); }
invocation (OUT) ::= expr (LHS) MODLIKE (OP) expr (RHS). { OUT = seg_parse_binop(LHS, OP, RHS); }
invocation (OUT) ::= expr (LHS) EXPLIKE (OP) expr (RHS). { OUT = seg_parse_binop(LHS, OP, RHS); }

// Unary operators

invocation ::= NOTLIKE expr.

// Paren method call, explicit receiver

invocation (OUT) ::= expr (R) PERIOD METHODNAME (MN) commaargs (ARGS) RPAREN.
{
  seg_arg_list *args = seg_reverse_args(ARGS);
  OUT = seg_parse_methodcall(R, MN, 1, args);
}

// Paren method call, implicit receiver

invocation (OUT) ::= METHODNAME (MN) commaargs (ARGS) RPAREN.
{
  seg_arg_list *args = seg_reverse_args(ARGS);
  OUT = seg_parse_methodcall(seg_implicit_self(), MN, 1, args);
}

// Space method call, no arguments.

invocation (OUT) ::= expr (R) PERIOD IDENTIFIER (SEL).
{
  OUT = seg_parse_methodcall(R, SEL, 0, NULL);
}

// Space method call, explicit receiver

spaceinvocation (OUT) ::= expr (R) PERIOD IDENTIFIER (SEL) spaceargs (ARGS).
{
  seg_arg_list *args = seg_reverse_args(ARGS);
  OUT = seg_parse_methodcall(R, SEL, 0, args);
}

// Space method call, implicit receiver

spaceinvocation (OUT) ::= IDENTIFIER (SEL) spaceargs (ARGS).
{
  seg_arg_list *args = seg_reverse_args(ARGS);
  OUT = seg_parse_methodcall(seg_implicit_self(), SEL, 0, args);
}

// Argument lists.

commaargs ::=.
commaargs (OUT) ::= commaarg (IN). { OUT = IN; }
commaargs (OUT) ::= commaargs (LIST) COMMA commaarg (NEW).
{
  NEW->next = LIST;
  OUT = NEW;
}

commaarg (OUT) ::= statement (V).
{
  OUT = seg_parse_arg(V, NULL);
}

commaarg (OUT) ::= KEYWORD (KW) statement (V).
{
  OUT = seg_parse_arg(V, KW);
}

spaceargs (OUT) ::= spacearg (IN). { OUT = IN; }
spaceargs (OUT) ::= spaceargs (LIST) spacearg (NEW).
{
  NEW->next = LIST;
  OUT = NEW;
}

spacearg (OUT) ::= expr (V).
{
  OUT = seg_parse_arg(V, NULL);
}

spacearg (OUT) ::= KEYWORD (KW) expr (V).
{
  OUT = seg_parse_arg(V, KW);
}
