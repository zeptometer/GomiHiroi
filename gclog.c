#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

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

void
logsocket(const char* format, ...)
{
  char buf[100];
  int len;
  va_list args;

  va_start(args, format);
  len = vsnprintf(buf, 100, format, args);
  va_end(args);

  write(clientsocket, buf, len);
  write(clientsocket, "", 1);
}

void
logAlloc(void* ptr, PTR_TYPE type)
{
  logsocket("ALLOC : %s : %p", typename(type), ptr);
}

void
logRef(void* from, void* to)
{
  logsocket("REF : %p : %p", from, to);
}

void
logDeref(void* from, void* to)
{
  logsocket("DEREF : %p : %p", from, to);
}

void
logMark(void* ptr)
{
  logsocket("MARK : %p", ptr);
}

void
logSweep()
{
  logsocket("SWEEP");
}
