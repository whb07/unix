//
// Created by wbright on 8/22/21.
//

#ifndef UNIX_SRC_CLOCK_H_
#define UNIX_SRC_CLOCK_H_

#include "conf.h"

#define UMODE 0170000
#define SCHMAG 10

void incupc(int *pc, int *u_prof);
void display(void);
void clock(DeviceCode *dev, int sp, int r1, int nps, int *r0, int *pc, int ps);
void timeout(int *fun, int arg, int tim);
#endif // UNIX_SRC_CLOCK_H_
