#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "errorutil.h"
#include "object.h"

void
elog (const enum errorlevel l, const char* format, ...)
{
	va_list args;

	if (l == LOG) {
	  fprintf(stderr,"LOG: ");
	} else {
	  fprintf(stderr,"ERROR: ");
	}

	va_start(args, format);
	vfprintf(stderr, format, args);
	fprintf(stderr,"\n");
	va_end(args);

	if (l == ERROR) {
	  longjmp(toplevel, 1);
	}
}

static void
getTypeName (char* str, KrtType type)
{
  switch  (type) {
  case KRT_EMPTY_LIST:
    strcpy(str,"KRT_EMPTY_LIST");
    break;
  case KRT_CONS:
    strcpy(str,"KRT_CONS");
    break;
  case KRT_SYMBOL:
    strcpy(str,"KRT_SYMBOL");
    break;
  case KRT_NUMBER:
    strcpy(str,"KRT_NUMBER");
    break;
  case KRT_BOOL:
    strcpy(str,"KRT_BOOL");
    break;
  case KRT_CLOSURE:
    strcpy(str, "KRT_CLOSURE");
    break;
  case KRT_PRIM_FUNC:
    strcpy(str, "KRT_PRIM_FUNC");
    break;
  case KRT_SYNTAX:
    strcpy(str, "KRT_SYNTAX");
    break;
  }
}

int getArglen (KrtObj args)
{
  int n = 0;
  while (getKrtType(args) != KRT_EMPTY_LIST) {
    if (getKrtType(args) != KRT_CONS) {
      elog(ERROR, "sexp must be list");
    }
    n++;
    args = getCdr(args);
  }
  return n;
}

void
assertArity (int len, int isVariable, KrtObj args)
{
  int arglen = getArglen(args);
  if (!((len == arglen) || (isVariable && len < arglen)))
    elog(ERROR, "wrong argument numbers");
}

void
assertType (KrtType type, KrtObj obj)
{
  if (type != getKrtType(obj)) {
    char expect[10];
    char acquire[10];

    getTypeName(expect, type);
    getTypeName(acquire, getKrtType(obj));

    elog(ERROR, "expect %s, but acquired %s", expect, acquire);
  }
}
