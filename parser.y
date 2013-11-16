%code requires {
#include "object.h"

int yylex (void);
void yyerror (char const *);
}

%code {
#include "common.h"
#include "eval.h"
#include "print.h"
#include "errorutil.h"
#include "gclog.h"
}

%define api.value.type {KrtObj}
%token ATOM

%%

input
:
| input sexp { if (setjmp(toplevel) == 0) {
                 currentCode = $2;
                 printKrtObj(eval($2, rootEnv));
               } else {
                 printf("#<error detected>");
               }
	       printf("\n");
	       currentCode = makeKrtEmptyList();
	       collectGarbage();
               printf("scm> "); }
;

sexp
: ATOM { $$ = $1; }
| '\'' sexp { $$ = makeKrtCons(makeKrtSymbol("quote"),
			       makeKrtCons($2, makeKrtEmptyList())); }
| list { $$ = $1; }
;

list
: '(' ')' { $$ = makeKrtEmptyList(); }
| '(' list_item ')' { $$ = $2; }
;

list_item
: sexp list_item { $$ = makeKrtCons($1, $2); }
| sexp '.' sexp  { $$ = makeKrtCons($1, $3); }
| sexp           { $$ = makeKrtCons($1, makeKrtEmptyList()); }
;

%%

void yyerror(char const *s)
{
  printf("%s\n",s);
}

int main()
{
  init_socket(5001);
  initialize();
  printf("scm> ");
  yyparse();
  end_socket();
  return 0;
}
