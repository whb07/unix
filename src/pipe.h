//
// Created by wbright on 8/22/21.
//

#ifndef UNIX_SRC_PIPE_H_
#define UNIX_SRC_PIPE_H_

void prele(int *ip);
void pipe(void);

void readp(int *fp);
void plock(int *ip);
void writep(int *fp);

#endif // UNIX_SRC_PIPE_H_
