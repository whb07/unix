//
// Created by wbright on 8/22/21.
//

#ifndef UNIX_SRC_IGET_H_
#define UNIX_SRC_IGET_H_
#include "conf.h"
#include "inode.h"
#include <stdint.h>

struct inode *iget(DeviceCode *dev, int ino);

void iput(struct inode *p);

void iupdat(int *p, int *tm);
void iput(struct inode *p);
void itrunc(int *ip);
struct inode *maknode(int mode);
void wdir(struct inode *ip);
#endif // UNIX_SRC_IGET_H_
