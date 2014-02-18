#include "common.h"
#include "object.h"
#include "intern.h"

static int n_syms;
static char* sym[100];

char*
internString (char* name)
{
  int i;
  for (i=0; i<n_syms; i++) {
    if(strcmp(sym[i], name) == 0) {
      return sym[i];
    }
  }

  sym[n_syms] = malloc(sizeof(char)*(1+strlen(name)));
  strcpy(sym[n_syms], name);

  if(n_syms == 99) {
    printf("fee.. too many symbols");
  }

  return sym[n_syms++];
}
