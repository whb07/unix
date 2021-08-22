//
// Created by wbright on 8/22/21.
//

#ifndef UNIX_SRC_FIO_H_
#define UNIX_SRC_FIO_H_
#include <stdint.h>

intptr_t *getf(int f);
void closef(int *fp);
void closei(int *ip, int rw);
void openi(int *ip, int rw);
unsigned int access(int *aip, int mode);
struct inode *owner(void);
unsigned int suser(void);
int ufalloc(void);
struct file *falloc(void);
#endif // UNIX_SRC_FIO_H_
