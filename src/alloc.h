//
// Created by wbright on 8/22/21.
//

#ifndef UNIX_SRC_ALLOC_H_
#define UNIX_SRC_ALLOC_H_
#include "conf.h"
#include "inode.h"
#include <stdint.h>

void *alloc(DeviceCode *dev);
struct filsys *getfs(DeviceCode *dev);
int badblock(struct filsys *afp, char *abn, DeviceCode *dev);
void free(DeviceCode *dev, unsigned int bno);
intptr_t *ialloc(DeviceCode *dev);
void ifree(DeviceCode *dev, int ino);
void update(void);
void pipe(void);
#endif // UNIX_SRC_ALLOC_H_
