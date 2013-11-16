#ifndef GCLOG_H
#define GCLOG_H

void init_socket(int port);
void end_socket();

typedef enum {
  PTR_ENV,
  PTR_CONS,
  PTR_CLOSURE,
} PTR_TYPE;

void logAlloc(void* ptr, PTR_TYPE type);
void logRef(void* from, void* to);
void logDeref(void* from, void* to);
void logMark(void* ptr);
void logSweep();

#endif
