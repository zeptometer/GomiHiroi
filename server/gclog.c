#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdint.h>

#include "gclog.h"

static int serversocket, clientsocket;

void
gomihiroi_initialize(int portnum)
{
  struct sockaddr_in address;

  if ((serversocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    abort();
  }

  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port	  = htons(portnum);
  address.sin_family	  = AF_INET;

  if (bind(serversocket, (struct sockaddr*)&address, sizeof(address)) == -1) {
    abort();
  }

  if (listen(serversocket, 10) == -1) {
    abort();
  }

  clientsocket = accept(serversocket, NULL, NULL);
}

void
gomihiroi_finalize()
{
  shutdown(clientsocket, 2);
  close(clientsocket);
}

static const char ALLOC = 1;
static const char REF   = 2;
static const char DEREF = 3;
static const char MARK  = 4;
static const char SWEEP = 5;

void
gomihiroi_log_alloc(void* ptr, int typeid)
{
  write(clientsocket, &ALLOC, 1);
  write(clientsocket, &typeid, 4);
  write(clientsocket, &ptr, 8);
}

void
gomihiroi_log_ref(void* from, void* to)
{
  write(clientsocket, &REF, 1);
  write(clientsocket, &from, 8);
  write(clientsocket, &to,   8);
}

void
gomihiroi_log_deref(void* from, void* to)
{
  write(clientsocket, &DEREF, 1);
  write(clientsocket, &from,  8);
  write(clientsocket, &to,    8);
}

void
gomihiroi_log_mark(void* ptr)
{
  uint64_t val = (uint64_t)ptr;
  const static char mark = 1;
  write(clientsocket, &MARK, 1);
  write(clientsocket, &val,  8);
  write(clientsocket, &mark, 1);
}

void
gomihiroi_log_sweep()
{
  write(clientsocket, &SWEEP, 1);
}
