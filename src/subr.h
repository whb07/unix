//
// Created by wbright on 8/22/21.
//

#ifndef UNIX_SRC_SUBR_H_
#define UNIX_SRC_SUBR_H_
#include <stdint.h>
#include "inode.h"
void bcopy(intptr_t *from, intptr_t *to, unsigned int count);
intptr_t  * bmap(struct inode *ip, int bn);

int cpass(void);
void nodev(void);
void nulldev(void);
int passc(char c);

#endif // UNIX_SRC_SUBR_H_
