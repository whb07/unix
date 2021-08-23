//
// Created by wbright on 8/22/21.
//

#ifndef UNIX_SRC_SIG_H_
#define UNIX_SRC_SIG_H_

#include <stdint.h>

int issig(void);

void psig(void);
void psignal(int *p, int sig);
void signal(int tp, int sig);
void stop(void);
int core(void);
int grow(char* sp);
void ptrace(void);
int procxmt(void);
#endif // UNIX_SRC_SIG_H_
