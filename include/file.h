//
// Created by wbright on 8/22/21.
//

#ifndef UNIX_INCLUDE_FILE_H_
#define UNIX_INCLUDE_FILE_H_
#include "param.h"
/*
 * One file structure is allocated
 * for each open/creat/pipe call.
 * Main use is to hold the read/write
 * pointer associated with each open
 * file.
 */
struct file {
  char f_flag;
  char f_count;      /* reference count */
  int f_inode;       /* pointer to inode structure */
  char *f_offset[2]; /* read/write character pointer */
} file[NFILE];

/* flags */
#define FREAD 01
#define FWRITE 02
#define FPIPE 04

#endif // UNIX_INCLUDE_FILE_H_
