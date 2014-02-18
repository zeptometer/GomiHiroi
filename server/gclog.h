#ifndef GCLOG_H
#define GCLOG_H

void gomihiroi_initialize(int port);
void gomihiroi_finalize();

void gomihiroi_log_alloc(void* ptr, int typeid);
void gomihiroi_log_ref(void* from, void* to);
void gomihiroi_log_deref(void* from, void* to);
void gomihiroi_log_mark(void* ptr);
void gomihiroi_log_sweep();

#endif
