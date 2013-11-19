#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdint.h>

#include "errorutil.h"
#include "gclog.h"

static char*
typename(PTR_TYPE type)
{
  char* name = "";

  if (type == PTR_ENV)
    name = "ENV";
  else if (type == PTR_CONS)
    name = "CONS";
  else if (type == PTR_CLOSURE)
    name = "CLOSURE";
  else
    elog(ERROR, "bad PTR_TYPE");

  return name;
}

static int serversocket, clientsocket;

void
init_socket(int portnum)
{
  struct sockaddr_in address;

  if ((serversocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    elog(FATAL, "could not make a socket");
  }

  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port	  = htons(portnum);
  address.sin_family	  = AF_INET;

  if (bind(serversocket, (struct sockaddr*)&address, sizeof(address)) == -1) {
    elog(FATAL, "could not connect to host");
  }

  if (listen(serversocket, 10) == -1) {
    elog(FATAL, "could not listen");
  }

  clientsocket = accept(serversocket, NULL, NULL);
}

void
end_socket()
{
  shutdown(clientsocket, 2);
  close(clientsocket);
}

static const char ALLOC = 0;
static const char REF   = 1;
static const char DEREF = 2;
static const char MARK  = 3;
static const char SWEEP = 4;

static const uint32_t ENV   = 0;
static const uint32_t CONS  = 1;
static const uint32_t CLOSURE = 2;

void
logAlloc(void* ptr, PTR_TYPE type)
{
  uint64_t val = (uint64_t)ptr;
  write(clientsocket, &ALLOC, 1);
  if (type == PTR_ENV)     write(clientsocket, &ENV,     4);
  if (type == PTR_CONS)    write(clientsocket, &CONS,    4);
  if (type == PTR_CLOSURE) write(clientsocket, &CLOSURE, 4);
  write(clientsocket, &val, 8);
  elog(LOG, "ALLOC {typeid: ??, addr: %x}", val);
}

void
logRef(void* from, void* to)
{
  uint64_t f = (uint64_t)from;
  uint64_t t = (uint64_t)to;
  write(clientsocket, &REF, 1);
  write(clientsocket, &from, 8);
  write(clientsocket, &to,   8);
  elog(LOG, "REF {from: %x to: %x}", f, t);
}

void
logDeref(void* from, void* to)
{
  uint64_t f = (uint64_t)from;
  uint64_t t = (uint64_t)to;
  write(clientsocket, &DEREF, 1);
  write(clientsocket, &from,  8);
  write(clientsocket, &to,    8);
  elog(LOG, "DEREF {from: %x to: %x}", f, t);
}

void
logMark(void* ptr)
{
  uint64_t val = (uint64_t)ptr;
  const static char mark = 1;
  write(clientsocket, &MARK, 1);
  write(clientsocket, &val,  8);
  write(clientsocket, &mark, 1);
  elog(LOG, "MARK {addr: %x, stat: %d}", val, mark);
}

void
logSweep()
{
  write(clientsocket, &SWEEP, 1);
  elog(LOG, "SWEEP");
}
