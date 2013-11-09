#include <stdlib.h>
#include "object.h"
#include "errorutil.h"

#define MAX_N_OBJ 1<<20

int    n_objPool = 0;
KrtObj objPool[MAX_N_OBJ];
int    n_envPool = 0;
KrtEnv envPool[MAX_N_OBJ];

/*
 * struct declaration: this is also declared in
 * object.c. This is need for deciding data size
 */

typedef struct {
  KrtObj car;
  KrtObj cdr;
} KrtCons;

typedef struct {
  KrtEnv env;
  KrtObj args;
  KrtObj code;
} KrtClosure;

typedef struct KrtVarData *KrtVar;
struct KrtVarData {
  KrtVar next;
  KrtObj symbol;
  KrtObj value;
};

struct KrtEnvData {
  KrtEnv parent;
  KrtVar head;
};


KrtObj
allocKrtObj(KrtType type)
{
  KrtObj obj;

  switch (type) {
  case KRT_CONS:
    obj.val.ptr = malloc(sizeof(KrtCons));
    break;
  case KRT_CLOSURE:
    obj.val.ptr = malloc(sizeof(KrtClosure));
    break;
  default:
    elog(LOG, "not need allocate");
  }

  obj.type = type;
  objPool[n_objPool++] = obj;
  return obj;
}

KrtEnv
allocKrtEnv()
{
  KrtEnv env = malloc(sizeof(struct KrtEnvData));
  envPool[n_envPool++] = env;
  return env;
}

KrtObj
markAndSweep()
{
  
}
