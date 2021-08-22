//
// Created by wbright on 8/22/21.
//

#ifndef UNIX_SRC_NAMI_H_
#define UNIX_SRC_NAMI_H_
#include "inode.h"

struct inode *namei(int (*func)(), unsigned int flag);
#endif // UNIX_SRC_NAMI_H_
