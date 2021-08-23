//
// Created by wbright on 8/22/21.
//

#ifndef UNIX_SRC_NAMI_H_
#define UNIX_SRC_NAMI_H_
#include "inode.h"
#include <stdint.h>

struct inode *namei(int (*func)(), unsigned int flag);
int fubyte(const void *base);
intptr_t schar(void) ;

int uchar(void);
#endif // UNIX_SRC_NAMI_H_
