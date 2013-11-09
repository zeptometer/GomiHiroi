#include <gc.h>
#include "common.h"
#include "object.h"
#include "intern.h"
#include "print.h"
#include "errorutil.h"
#include "gc.h"

/* KrtObj */

typedef struct {
  KrtObj car;
  KrtObj cdr;
} KrtCons;

typedef struct {
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

  cell->car = car;
  cell->cdr = cdr;

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
  KrtClosure *ptr = GC_malloc(sizeof(KrtClosure));
  KrtObj      obj;

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
  KrtEnv parent;
  KrtVar head;
};


KrtEnv
makeKrtEnv (KrtEnv parent)
{
  KrtEnv env = allocKrtEnv();

  env->parent = parent;
  env->head   = NULL;

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
}

void
bindVar (KrtObj sym, KrtObj val, KrtEnv env)
{
  KrtVar var = GC_malloc(sizeof(struct KrtVarData));
  
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
        curvar->value = val;
        return;
      }

      curvar = curvar->next;
    }

    curframe = curframe->parent;
  }

  abort();
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
