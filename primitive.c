#include "common.h"
#include "object.h"
#include "print.h"
#include "eval.h"
#include "errorutil.h"

static KrtObj
KrtPrimCar (KrtObj args)
{
  assertArity(1, false, args);
  assertType(KRT_CONS, getCar(args));

  return getCar(getCar(args));
}

static KrtObj
KrtPrimCdr (KrtObj args)
{
  assertArity(1, false, args);
  assertType(KRT_CONS, getCar(args));

  return getCdr(getCar(args));
}

static KrtObj
KrtPrimCons (KrtObj args)
{
  KrtObj car, cdr;

  assertArity(2, false, args);

  car = getCar(args);
  cdr = getCar(getCdr(args));

  return makeKrtCons(car, cdr);
}

static KrtObj
KrtPrimList (KrtObj args)
{
  return args;
}

static KrtObj
KrtPrimIsEmpty (KrtObj args)
{
  assertArity(1, false, args);
  return makeKrtBool(getKrtType(getCar(args)) == KRT_EMPTY_LIST);
}

static KrtObj
KrtPrimIsPair (KrtObj args)
{
  assertArity(1, false, args);
  return makeKrtBool(getKrtType(args) == KRT_CONS);
}

static KrtObj
KrtPrimIsSymbol (KrtObj args)
{
  assertArity(1, false, args);
  return makeKrtBool(getKrtType(args) == KRT_SYMBOL);
}

static KrtObj
KrtPrimPlus (KrtObj args)
{
  double ans = 0;
  KrtObj num;
  KrtObj rest = args;

  while (getKrtType(rest) != KRT_EMPTY_LIST) {
    num  = getCar(rest);
    rest = getCdr(rest);

    assertType(KRT_NUMBER, num);

    ans += getNum(num);
  }

  return makeKrtNumber(ans);
}

static KrtObj
KrtPrimPrint (KrtObj args)
{
  assertArity(1, false, args);
  printKrtObj(getCar(args));
  return makeKrtEmptyList();
}


static KrtObj
KrtPrimSub (KrtObj args)
{
  assertArity(1, true, args);

  if (getKrtType(getCdr(args)) == KRT_EMPTY_LIST) {
    return makeKrtNumber(-getNum(getCar(args)));
  } else {
    double ans = getNum(getCar(args));
    KrtObj num;
    KrtObj rest = getCdr(args);

    while (getKrtType(rest) != KRT_EMPTY_LIST) {
      num  = getCar(rest);
      rest = getCdr(rest);

      assertType(KRT_NUMBER, num);

      ans -= getNum(num);
    }

    return makeKrtNumber(ans);
  }
}

static KrtObj
KrtPrimIsEq (KrtObj args)
{
  assertArity(2, false, args);
  return makeKrtBool(getCar(args).val.ptr == getCar(getCdr(args)).val.ptr);
}

static int
isEqv (KrtObj a, KrtObj b)
{
  if (getKrtType(a) != getKrtType(b)) {
    return 0;
  } else {
    switch (getKrtType(a)) {
    case KRT_EMPTY_LIST:
      return 1;
    case KRT_CONS:
      return (a.val.ptr == b.val.ptr)
        || (isEqv(getCar(a), getCar(b))
         && isEqv(getCdr(a), getCdr(b)));
    case KRT_SYMBOL:
    case KRT_CLOSURE:
    case KRT_PRIM_FUNC:
      return a.val.ptr == b.val.ptr;
    case KRT_NUMBER:
      return getNum(a) == getNum(b);
    case KRT_BOOL:
      return getBool(a) == getBool(b);
    default:
      abort();
    }
  }
}

static KrtObj
KrtPrimIsEqv (KrtObj args)
{
  assertArity(2, false, args);
  return makeKrtBool(isEqv(getCar(args), getCar(getCdr(args))));
}

void
defineKrtPrimFunc (char* name, KrtPrimFunc func)
{
  KrtObj sym  = makeKrtSymbol(name);
  KrtObj prim = makeKrtPrimFunc(func);

  bindVar(sym, prim, rootEnv);
}

void
defineKrtSyntax (char* name, KrtSyntaxType tag)
{
  KrtObj sym    = makeKrtSymbol(name);
  KrtObj syntax = makeKrtSyntax(tag);

  bindVar(sym, syntax, rootEnv);
}

void initialize_primitive()
{
  defineKrtSyntax("quote",  KRT_SYNTAX_QUOTE);
  defineKrtSyntax("if",     KRT_SYNTAX_IF);
  defineKrtSyntax("lambda", KRT_SYNTAX_LAMBDA);
  defineKrtSyntax("begin",  KRT_SYNTAX_BEGIN);
  defineKrtSyntax("set!",   KRT_SYNTAX_SET);
  defineKrtSyntax("define",  KRT_SYNTAX_DEFINE);

  defineKrtPrimFunc("car",    KrtPrimCar);
  defineKrtPrimFunc("cdr",    KrtPrimCdr);
  defineKrtPrimFunc("cons",   KrtPrimCons);
  defineKrtPrimFunc("list",   KrtPrimList);
  defineKrtPrimFunc("null?",  KrtPrimIsEmpty);
  defineKrtPrimFunc("pair?",  KrtPrimIsPair);
  defineKrtPrimFunc("symbol?",KrtPrimIsSymbol);
  defineKrtPrimFunc("+",      KrtPrimPlus);
  defineKrtPrimFunc("-",      KrtPrimSub);
  defineKrtPrimFunc("print",  KrtPrimPrint);
  defineKrtPrimFunc("eq?",    KrtPrimIsEq);
  defineKrtPrimFunc("eqv?",   KrtPrimIsEqv);
}
