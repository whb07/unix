//
// Created by wbright on 8/22/21.
//

#ifndef UNIX_SRC_SLP_H
#define UNIX_SRC_SLP_H
#include "proc.h"
#include "stdint.h"

void wakeup(intptr_t *chan);
void sleep(intptr_t *chan, unsigned int pri);
void setpri(struct proc *up);
void setrun(struct proc *p);
void expand(int newsize);
int swtch(void);
#endif // UNIX_SRC_SLP_H
