#include <setjmp.h>
#include "object.h"

#ifndef ERRORUTIL_H
#define ERRORUTIL_H

jmp_buf toplevel;

enum errorlevel {
  LOG,
  ERROR,
};

void elog(enum errorlevel l, const char* format, ...);

int getArgLen(KrtObj args);

void assertArity (int len, int isVariable, KrtObj args);
void assertType (KrtType type, KrtObj obj);

#endif
