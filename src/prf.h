//
// Created by wbright on 8/22/21.
//

#ifndef UNIX_SRC_PRF_H
#define UNIX_SRC_PRF_H
#include "buf.h"
void panic(char *s);
void prdev(char *str, DeviceCode *dev);
void printf(char fmt[],...);
void deverror(int *bp, int o1, int o2);
void putchar(char *c);
void prdev(char *str, DeviceCode *dev);
void printn(int n, int b);
#endif // UNIX_SRC_PRF_H
