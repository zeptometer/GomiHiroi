#include "common.h"
#include "object.h"
#include "intern.h"
#include "print.h"
#include "gclog.h"
#include "errorutil.h"

/* gc allocate fucntinon */
static KrtObj allocKrtObj(KrtType);
static KrtEnv allocKrtEnv();

/* KrtObj */

typedef struct {
  byte   mark;
  KrtObj car;
  KrtObj cdr;
} KrtCons;

typedef struct {
  byte   mark;
  KrtEnv env;
  KrtObj args;
  KrtObj code;
} KrtClosure;

/* constructor */

KrtObj
makeKrtEmptyList ()
{
  KrtObj obj;

  obj.type     = KRT_EMPTY_LIST;
  obj.val.ptr  = NULL;

  return obj;
}

KrtObj
makeKrtCons (KrtObj car, KrtObj cdr)
{
  KrtObj   obj  = allocKrtObj(KRT_CONS);
  KrtCons *cell = (KrtCons *)obj.val.ptr; 

  cell->mark = false;
  cell->car  = car;
  cell->cdr  = cdr;

  obj.type    = KRT_CONS;
  obj.val.ptr = (void*)cell;

  return obj;
}

KrtObj
makeKrtSymbol (char* name)
{
  KrtObj obj;
  char*  sym = internString(name);

  obj.type    = KRT_SYMBOL;
  obj.val.ptr = (void*)sym;
  
  return obj;
}

KrtObj
makeKrtNumber (double val)
{
  KrtObj obj;
 
  obj.type     = KRT_NUMBER;
  obj.val.num  = val;

  return obj;
}

KrtObj
makeKrtBool (int val)
{
  KrtObj obj;

  obj.type     = KRT_BOOL;
  obj.val.bool = val;

  return obj;
}

KrtObj
makeKrtClosure (KrtEnv env, KrtObj args, KrtObj code)
{
  KrtObj      obj = allocKrtObj(KRT_CLOSURE);
  KrtClosure *ptr = (KrtClosure *)obj.val.ptr;

  ptr->mark = false;
  ptr->env  = env;
  ptr->args = args;
  ptr->code = code;

  obj.type     = KRT_CLOSURE;
  obj.val.ptr  = (void*)ptr;

  return obj;
}

KrtObj
makeKrtPrimFunc (KrtPrimFunc func)
{
  KrtObj obj;

  obj.type    = KRT_PRIM_FUNC;
  obj.val.ptr = (void*)func;

  return obj;
}

KrtObj
makeKrtSyntax (KrtSyntaxType syntax)
{
  KrtObj obj;

  obj.type       = KRT_SYNTAX;
  obj.val.syntax = syntax;

  return obj;
}

/* accessor */

KrtType
getKrtType (KrtObj obj)
{
  return obj.type;
}

// cons
KrtObj
getCar (KrtObj cons)
{
  return ((KrtCons*)cons.val.ptr)->car;
}

KrtObj
getCdr (KrtObj cons)
{
  return ((KrtCons*)cons.val.ptr)->cdr;
}

// symbol
char*
getName (KrtObj sym)
{
  return (char*)sym.val.ptr;
}

// number
double
getNum (KrtObj num)
{
  return (double)num.val.num;
}

// bool
int
getBool (KrtObj bool)
{
  return (int)bool.val.bool;
}

// closure | syntactic closure
KrtEnv
getEnv (KrtObj obj)
{
  return ((KrtClosure*)obj.val.ptr)->env;
}

KrtObj
getArgs (KrtObj obj)
{
  return ((KrtClosure*)obj.val.ptr)->args;
}

KrtObj
getCode (KrtObj obj)
{
  return ((KrtClosure*)obj.val.ptr)->code;
}

// primitive function
KrtPrimFunc
getPrimFunc (KrtObj obj)
{
  return (KrtPrimFunc)obj.val.ptr;
}

// syntax
KrtSyntaxType
getSyntaxType (KrtObj obj)
{
  return obj.val.syntax;
}

/* env */

typedef struct KrtVarData *KrtVar;
struct KrtVarData {
  KrtVar next;
  KrtObj symbol;
  KrtObj value;
};

struct KrtEnvData {
  byte   mark;
  KrtEnv parent;
  KrtVar head;
};


KrtEnv
makeKrtEnv (KrtEnv parent)
{
  KrtEnv env = allocKrtEnv();

  env->mark   = false;
  env->parent = parent;
  env->head   = NULL;

  logRef(env, parent);

  return env;
}

KrtObj
getVar (KrtObj sym, KrtEnv env)
{
  KrtEnv curframe = env;

  while (curframe != NULL) {
    KrtVar curvar = curframe->head;
    
    while (curvar != NULL) {
      if (sym.val.ptr == curvar->symbol.val.ptr)
        return curvar->value;
      
      curvar = curvar->next;
    }

    curframe = curframe->parent;
  }

  elog(ERROR, "variable %s not found", getName(sym));
  return makeKrtEmptyList();
}

void
bindVar (KrtObj sym, KrtObj val, KrtEnv env)
{
  KrtVar var = malloc(sizeof(struct KrtVarData));

  if (val.type == KRT_CONS || val.type == KRT_CLOSURE)
    logRef(env, val.val.ptr);

  var->symbol = sym;
  var->value  = val;
  var->next   = env->head;

  env->head = var;
}

void
setVar (KrtObj sym, KrtObj val, KrtEnv env)
{
  KrtEnv curframe = env;

  while (curframe != NULL) {
    KrtVar curvar = curframe->head;
    
    while (curvar != NULL) {
      if (sym.val.ptr == curvar->symbol.val.ptr) {
	if (curvar->value.type == KRT_CONS || curvar->value.type == KRT_CLOSURE)
	  logDeref(curframe, curvar->value.val.ptr);

	if (val.type == KRT_CONS || val.type == KRT_CLOSURE)
	  logRef(curframe, val.val.ptr);

        curvar->value = val;
        return;
      }

      curvar = curvar->next;
    }

    curframe = curframe->parent;
  }

  elog(ERROR, "variable %s not defined", getName(sym));
}

void
printEnv (KrtEnv env)
{
  KrtEnv curframe = env;

  printf("\n");
  while (curframe != NULL) {
    KrtVar curvar = curframe->head;
    
    while (curvar != NULL) {
      printKrtObj(curvar->symbol);
      printf("\t");
      printKrtObj(curvar->value);
      printf("\n");
      
      curvar = curvar->next;
    }
    
    printf("|\nv\n");

    curframe = curframe->parent;
  }
  printf("\n");
}


/* garbage collection */

#define MAX_STACK_DEPTH 1000

int     n_envStack = 0;
KrtEnv  envStack[MAX_STACK_DEPTH];

void
pushEnv(KrtEnv env)
{
  if (n_envStack == MAX_STACK_DEPTH)
    elog(ERROR, "stack overflow");

  envStack[n_envStack++] = env;
}

void
popEnv()
{
  if (n_envStack == 1)
    elog(ERROR, "stack corrupted");

  n_envStack--;
}

void
resetEnvStack()
{
  n_envStack = 1;
}

#define MAX_N_OBJ 1000000

int    n_objPool = 0;
KrtObj objPool[MAX_N_OBJ];
int    n_envPool = 0;
KrtEnv envPool[MAX_N_OBJ];

static KrtObj
allocKrtObj(KrtType type)
{
  KrtObj obj;

  if (n_objPool == MAX_N_OBJ)
    collectGarbage();
  if (n_objPool == MAX_N_OBJ)
    elog(ERROR, "not enough memory");

  switch (type) {
  case KRT_CONS:
    obj.val.ptr = malloc(sizeof(KrtCons));
    logAlloc(obj.val.ptr, PTR_CONS);
    break;
  case KRT_CLOSURE:
    obj.val.ptr = malloc(sizeof(KrtClosure));
    logAlloc(obj.val.ptr, PTR_CLOSURE);
    break;
  default:
    elog(LOG, "not need allocate");
  }

  obj.type = type;
  objPool[n_objPool++] = obj;
  return obj;
}

static KrtEnv
allocKrtEnv()
{
  KrtEnv env = malloc(sizeof(struct KrtEnvData));
  logAlloc(env, PTR_ENV);

  if (n_envPool == MAX_N_OBJ)
    collectGarbage();
  if (n_envPool == MAX_N_OBJ)
    elog(ERROR, "not enough memory");

  envPool[n_envPool++] = env;
  return env;
}

static void
markObj (KrtObj obj)
{
  switch (getKrtType(obj)) {
  case KRT_CONS:
    ((KrtCons*)obj.val.ptr)->mark = true;
    logMark(obj.val.ptr);
    break;
  case KRT_CLOSURE:
    ((KrtClosure*)obj.val.ptr)->mark = true;
    logMark(obj.val.ptr);
    break;
  default:
    elog(ERROR, "non-markable object");
  }
}

static void
unmarkObj (KrtObj obj)
{
  switch (getKrtType(obj)) {
  case KRT_CONS:
    ((KrtCons*)obj.val.ptr)->mark = false;
    break;
  case KRT_CLOSURE:
    ((KrtClosure*)obj.val.ptr)->mark = false;
    break;
  default:
    elog(ERROR, "non-markable object");
  }
}

static int
isMarkedObj (KrtObj obj)
{
  switch (getKrtType(obj)) {
  case KRT_CONS:
    return ((KrtCons*)obj.val.ptr)->mark;
  case KRT_CLOSURE:
    return ((KrtClosure*)obj.val.ptr)->mark;
  default:
    return false;
  }
}

static void
markEnv (KrtEnv env)
{
  logMark(env);
  env->mark = true;
}

static void
unmarkEnv (KrtEnv env)
{
  env->mark = false;
}

static byte
isMarkedEnv (KrtEnv env)
{
  return env->mark;
}

void scanObj (KrtObj obj);
void scanEnv (KrtEnv env);

void
scanCons (KrtObj cons) {
  markObj(cons);
  scanObj(getCar(cons));
  scanObj(getCdr(cons));
}

void
scanClosure (KrtObj closure) {
  markObj(closure);
  scanObj(getArgs(closure));
  scanObj(getCode(closure));
  scanEnv(getEnv(closure));
}

void
scanObj (KrtObj obj) {
  if (isMarkedObj(obj))
    return;

  switch (getKrtType(obj)) {
  case KRT_CONS:
    scanCons(obj);
    break;
  case KRT_CLOSURE:
    scanClosure(obj);
    break;
  default:
    break;
  }
}

void
scanEnv (KrtEnv env){
  while (env != NULL && !isMarkedEnv(env)) {
    KrtVar var    = env->head;
    markEnv(env);

    while (var != NULL) {
      KrtObj val = var->value;

      if (!isMarkedObj(val)) scanObj(val);
      var = var->next;
    }

    env = env->parent;
  }
}

extern KrtEnv rootEnv;

void
collectGarbage()
{
  int i,j;

  scanObj(currentCode);
  for (i=0; i<n_envStack; i++)
    scanEnv(envStack[i]);

  for (i=j=0; i<n_objPool; i++) {
    if (isMarkedObj(objPool[i])) {
      objPool[j++] = objPool[i];
      unmarkObj(objPool[i]);
    } else {
      free(objPool[i].val.ptr);
    }
  }
  elog(LOG, "free %d obj - holding %d obj", n_objPool-j, j);
  n_objPool = j;

  logSweep();
  for (i=j=0; i<n_envPool; i++) {
    if (isMarkedEnv(envPool[i])) {
      envPool[j++] = envPool[i];
      unmarkEnv(envPool[i]);
    } else {
      KrtVar var    = envPool[i]->head;

      while (var != NULL) {
	KrtVar next = var->next;
	free(var);
	var = next;
      }

      free(envPool[i]);
    }
  }
  elog(LOG, "free %d env - holding %d env", n_envPool-j, j);
  n_envPool = j;
}
