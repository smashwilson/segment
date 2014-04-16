%include {
  #include <assert.h>
  #include <stdlib.h>
  #include <stdio.h>

  #include "ast.h"
  #include "token.h"
  #include "parse_helpers.h"
  #include "symboltable.h"

  #define INTERN(name, length) seg_symboltable_intern(state->symboltable, name, length)
}

// Grammar definition for segment.

// Operators and operator precedence.

%left STRINGMID.
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
%type interpolated { seg_expr_node* }
%type interpolatedmiddle { seg_expr_node* }
%type invocation { seg_expr_node* }
%type spaceinvocation { seg_expr_node* }
%type blockstart { seg_expr_node* }
%type block { seg_expr_node* }

%type parameters { seg_parameter_list* }
%type commaparams { seg_parameter_list* }
%type parameter { seg_parameter_list* }

%type commaargs { seg_arg_list* }
%type commaarg  { seg_arg_list* }
%type spaceargs { seg_arg_list* }
%type spacearg  { seg_arg_list* }

%extra_argument { seg_parser_state *state }

// Grammar definition.

program (OUT) ::= statementlist (LIST).
{
  state->root = LIST;
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
  int value = seg_token_as_integer(L);
  seg_delete_token(L);

  OUT = malloc(sizeof(seg_expr_node));
  OUT->child_kind = SEG_INTEGER;
  OUT->child.integer.value = value;
}

expr ::= FLOAT.
expr ::= TRUE.
expr ::= FALSE.
expr ::= SYMBOL.

// Strings

expr (OUT) ::= STRING (S).
{
  /* ' ... ' or " ... " */
  size_t length;
  char *value = seg_token_without(S, 1, 1, &length);
  seg_delete_token(S);

  OUT = malloc(sizeof(seg_expr_node));
  OUT->child_kind = SEG_STRING;
  OUT->child.string.value = value;
  OUT->child.string.length = length;
}

expr (OUT) ::= interpolated (IN). { OUT = IN; }

interpolated (OUT) ::= STRINGSTART (START) interpolatedmiddle (MID) STRINGEND (END).
{
  /* " ... #{ */
  size_t start_length;
  char *start_content = seg_token_without(START, 1, 2, &start_length);
  seg_delete_token(START);

  seg_expr_node *receiver = malloc(sizeof(seg_expr_node));
  receiver->child_kind = SEG_STRING;
  receiver->child.string.value = start_content;
  receiver->child.string.length = start_length;

  MID->child.methodcall.receiver = receiver;

  /* } ... " */
  size_t end_length;
  char *end_content = seg_token_without(END, 1, 1, &end_length);
  seg_delete_token(END);

  if (end_content != NULL) {
    seg_expr_node *end_node = malloc(sizeof(seg_expr_node));
    end_node->child_kind = SEG_STRING;
    end_node->child.string.value = end_content;
    end_node->child.string.length = end_length;

    seg_arg_list *args = seg_parse_arg(state, end_node, NULL);
    args->next = MID->child.methodcall.args;
    MID->child.methodcall.args = args;
  }

  /* Arguments are reversed. */
  MID->child.methodcall.args = seg_reverse_args(MID->child.methodcall.args);

  OUT = MID;
}

interpolatedmiddle (OUT) ::= statement (E).
{
  OUT = seg_implicit_methodcall(state, NULL, SEG_METHOD_STRINGAPPEND);

  if (E->child_kind != SEG_STRING) {
    seg_expr_node *converter = seg_implicit_methodcall(state, E, SEG_METHOD_STRINGCONV);
    OUT->child.methodcall.args = seg_parse_arg(state, converter, NULL);
  } else {
    OUT->child.methodcall.args = seg_parse_arg(state, E, NULL);
  }
}

interpolatedmiddle (OUT) ::= interpolatedmiddle (PRE) STRINGMID (S) statement (E).
{
  /* } .. #{ */
  size_t mid_length;
  char *mid_content = seg_token_without(S, 1, 2, &mid_length);

  if (mid_content != NULL) {
    seg_expr_node *mid = malloc(sizeof(seg_expr_node));
    mid->child_kind = SEG_STRING;
    mid->child.string.value = mid_content;
    mid->child.string.length = mid_length;

    seg_arg_list *args = seg_parse_arg(state, mid, NULL);
    args->next = PRE->child.methodcall.args;
    PRE->child.methodcall.args = args;
  }

  if (E->child_kind != SEG_STRING) {
    seg_expr_node *converter = seg_implicit_methodcall(state, E, SEG_METHOD_STRINGCONV);

    seg_arg_list *args = seg_parse_arg(state, converter, NULL);
    args->next = PRE->child.methodcall.args;
    PRE->child.methodcall.args = args;
  } else {
    seg_arg_list *args = seg_parse_arg(state, E, NULL);
    args->next = PRE->child.methodcall.args;
    PRE->child.methodcall.args = args;
  }

  OUT = PRE;
}

// Compound Expressions

expr (OUT) ::= LPAREN statement (IN) RPAREN. { OUT = IN; }
expr (OUT) ::= IDENTIFIER (V).
{
  size_t length;
  const char *name = seg_token_as_string(V, &length);
  seg_delete_token(V);

  OUT = malloc(sizeof(seg_expr_node));

  if (seg_parser_isarg(state, name, length)) {
    OUT->child_kind = SEG_VAR;
    OUT->child.var.varname = INTERN(name, length);
  } else {
    OUT->child_kind = SEG_METHODCALL;
    OUT->child.methodcall.selector = INTERN(name, length);
    OUT->child.methodcall.receiver = seg_implicit_self(state);
    OUT->child.methodcall.args = NULL;
  }
}

expr (OUT) ::= block (IN). { OUT = IN; }

expr ::= assignment.

expr (OUT) ::= invocation (I). { OUT = I; }

// Blocks

block (OUT) ::= blockstart (BLK) parameters statementlist (BODY) BLOCKEND.
{
  OUT = BLK;
  OUT->child.block.body = BODY;

  /*
    Parameters are pushed in reverse order.
    This restores them to the correct order.
  */
  seg_parameter_list *params = seg_reverse_params(OUT->child.block.parameters);
  OUT->child.block.parameters = params;

  seg_parser_popcontext(state);
}

blockstart (OUT) ::= BLOCKSTART.
{
  OUT = malloc(sizeof(seg_expr_node));
  OUT->child_kind = SEG_BLOCK;
  OUT->child.block.parameters = NULL;
  OUT->child.block.body = NULL;

  seg_parser_pushcontext(state, &(OUT->child.block));
}

parameters ::= .
parameters ::= BAR commaparams BAR.

commaparams ::= parameter (IN).
{
  seg_parser_addparam(state, IN);
}

commaparams ::= commaparams COMMA parameter (NEW).
{
  seg_parser_addparam(state, NEW);
}

parameter (OUT) ::= IDENTIFIER (ID).
{
  char *name;
  size_t length;

  name = seg_token_as_string(ID, &length);
  seg_delete_token(ID);

  OUT = malloc(sizeof(seg_parameter_list));
  OUT->parameter = INTERN(name, length);
  OUT->next = NULL;
}

parameter ::= IDENTIFIER ASSIGNMENT expr.

// Assignment

assignment ::= lhs ASSIGNMENT expr.

lhs ::= IVAR.
lhs ::= TVAR.

// Invocation

// Binary operators

invocation (OUT) ::= expr (LHS) ANDLIKE (OP) expr (RHS).
{
  OUT = seg_parse_binop(state, LHS, OP, RHS);
}

invocation (OUT) ::= expr (LHS) ORLIKE (OP) expr (RHS).
{
  OUT = seg_parse_binop(state, LHS, OP, RHS);
}

invocation (OUT) ::= expr (LHS) PLUSLIKE (OP) expr (RHS).
{
  OUT = seg_parse_binop(state, LHS, OP, RHS);
}

invocation (OUT) ::= expr (LHS) MINUSLIKE (OP) expr (RHS).
{
  OUT = seg_parse_binop(state, LHS, OP, RHS);
}

invocation (OUT) ::= expr (LHS) MULTLIKE (OP) expr (RHS).
{
  OUT = seg_parse_binop(state, LHS, OP, RHS);
}

invocation (OUT) ::= expr (LHS) DIVLIKE (OP) expr (RHS).
{
  OUT = seg_parse_binop(state, LHS, OP, RHS);
}

invocation (OUT) ::= expr (LHS) MODLIKE (OP) expr (RHS).
{
  OUT = seg_parse_binop(state, LHS, OP, RHS);
}

invocation (OUT) ::= expr (LHS) EXPLIKE (OP) expr (RHS).
{
  OUT = seg_parse_binop(state, LHS, OP, RHS);
}

// Unary operators

invocation ::= NOTLIKE expr.

// Paren method call, explicit receiver

invocation (OUT) ::= expr (R) PERIOD METHODNAME (MN) commaargs (ARGS) RPAREN.
{
  seg_arg_list *args = seg_reverse_args(ARGS);
  OUT = seg_parse_methodcall(state, R, MN, 1, args);
}

// Paren method call, implicit receiver

invocation (OUT) ::= METHODNAME (MN) commaargs (ARGS) RPAREN.
{
  seg_arg_list *args = seg_reverse_args(ARGS);
  OUT = seg_parse_methodcall(state, seg_implicit_self(state), MN, 1, args);
}

// Space method call, no arguments.

invocation (OUT) ::= expr (R) PERIOD IDENTIFIER (SEL).
{
  OUT = seg_parse_methodcall(state, R, SEL, 0, NULL);
}

// Space method call, explicit receiver

spaceinvocation (OUT) ::= expr (R) PERIOD IDENTIFIER (SEL) spaceargs (ARGS).
{
  seg_arg_list *args = seg_reverse_args(ARGS);
  OUT = seg_parse_methodcall(state, R, SEL, 0, args);
}

// Space method call, implicit receiver

spaceinvocation (OUT) ::= IDENTIFIER (SEL) spaceargs (ARGS).
{
  seg_arg_list *args = seg_reverse_args(ARGS);
  OUT = seg_parse_methodcall(state, seg_implicit_self(state), SEL, 0, args);
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
  OUT = seg_parse_arg(state, V, NULL);
}

commaarg (OUT) ::= KEYWORD (KW) statement (V).
{
  OUT = seg_parse_arg(state, V, KW);
}

spaceargs (OUT) ::= spacearg (IN). { OUT = IN; }
spaceargs (OUT) ::= spaceargs (LIST) spacearg (NEW).
{
  NEW->next = LIST;
  OUT = NEW;
}

spacearg (OUT) ::= expr (V).
{
  OUT = seg_parse_arg(state, V, NULL);
}

spacearg (OUT) ::= KEYWORD (KW) expr (V).
{
  OUT = seg_parse_arg(state, V, KW);
}
