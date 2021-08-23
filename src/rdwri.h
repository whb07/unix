//
// Created by wbright on 8/22/21.
//

#ifndef UNIX_SRC_RDWRI_H_
#define UNIX_SRC_RDWRI_H_
#include "inode.h"

void writei(struct inode *aip);
void readi(struct inode *aip);
char * min(char *a, char *b);
char * max(char *a, char *b);
void iomove(int *bp, int o, int an, int flag);
#endif // UNIX_SRC_RDWRI_H_
