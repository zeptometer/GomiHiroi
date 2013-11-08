#include "common.h"
#include "object.h"
#include "primitive.h"
#include "print.h"
#include "errorutil.h"
#include "eval.h"

void
initialize ()
{
  rootEnv = makeKrtEnv(NULL);
  initialize_primitive();
}

static KrtObj
eval_each (KrtObj list, KrtEnv env)
{
  if (getKrtType(list) == KRT_EMPTY_LIST) {
    return makeKrtEmptyList();
  } else {
    return makeKrtCons(eval(getCar(list), env),
		       eval_each(getCdr(list), env));
  }
}

#define TAIL_CALL(c, e) do {			\
    code = c;					\
    env = e;					\
    goto LOOP;					\
  } while (0)					\


KrtObj
eval (KrtObj code, KrtEnv env)
{
 LOOP:
  switch (getKrtType(code)) {
  case KRT_SYMBOL:
    return getVar(code, env);

  case KRT_NUMBER:
  case KRT_BOOL:
    return code;

  case KRT_CONS:
    {
      KrtObj func = eval(getCar(code), env);
      KrtObj args = getCdr(code);

      switch (getKrtType(func)) {
      case KRT_CLOSURE:
	{
	  KrtEnv frame = makeKrtEnv(getEnv(func));

	  KrtObj sym;
	  KrtObj arg;
	  KrtObj restsym = getArgs(func);
	  KrtObj restarg = args;

	  while (getKrtType(restsym) != KRT_EMPTY_LIST) {
	    if (getKrtType(restsym) == KRT_SYMBOL) {
	      bindVar(restsym, eval_each(restarg, env), frame);
	      break;
	    }

	    arg     = getCar(restarg);
	    restarg = getCdr(restarg);
	    sym     = getCar(restsym);
	    restsym = getCdr(restsym);

	    bindVar(sym, eval(arg, env), frame);
	  }

	  TAIL_CALL(getCode(func), frame);
	}

      case KRT_PRIM_FUNC:
	return getPrimFunc(func)(eval_each(args, env));

      case KRT_SYNTAX:
	switch (getSyntaxType(func)) {
	case KRT_SYNTAX_QUOTE:
	  return getCar(getCdr(code));

	case KRT_SYNTAX_IF:
	  {
	    KrtObj pred = eval(getCar(args), env);

	    assertType(KRT_BOOL, pred);

	    if (getBool(pred)) {
	      TAIL_CALL(getCar(getCdr(args)), env);
	    } else {
	      KrtObj rest = getCdr(getCdr(args));
	      if (getKrtType(rest) == KRT_EMPTY_LIST) {
		return makeKrtEmptyList();
	      } else {
		TAIL_CALL(getCar(rest), env);
	      }
	    }
	  }

	case KRT_SYNTAX_LAMBDA:
	  {
	    KrtObj args = getCar(getCdr(code));
	    KrtObj body = getCdr(getCdr(code));

	    assertType(KRT_CONS, args);

	    if (getKrtType(getCdr(body)) == KRT_EMPTY_LIST)
	      return makeKrtClosure(env, args, getCdr(body));
	    else
	      return makeKrtClosure(env, args, makeKrtCons(makeKrtSymbol("begin"),body));
	  }

	case KRT_SYNTAX_BEGIN:
	  {
	    KrtObj body = getCdr(code);

	    while (getKrtType(getCdr(body)) != KRT_EMPTY_LIST) {
	      eval(getCar(body), env);
	      body = getCdr(body);
	    }

	    TAIL_CALL(getCar(body), env);
	  }

	case KRT_SYNTAX_SET:
	  {
	    KrtObj var = getCar(args);
	    KrtObj val = eval(getCar(getCdr(args)), env);

	    setVar(var, val, env);

	    return val;
	  }

	case KRT_SYNTAX_DEFINE:
	  {
	    KrtObj var = getCar(args);
	    KrtObj val = eval(getCar(getCdr(args)), env);

	    bindVar(var, val, env);

	    return var;
	  }
	}

      default:
	abort();
      }
    }
  default:
    abort();
  }
}
