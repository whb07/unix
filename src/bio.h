//
// Created by wbright on 8/21/21.
//

#ifndef UNIX_BIO_H
#define UNIX_BIO_H
#include <stdint.h>

#include "buf.h"
#include "conf.h"

struct buf *bread(DeviceCode *dev, intptr_t *blkno);

struct buf *breada(DeviceCode *adev, int blkno, int rablkno);

void bwrite(struct buf *bp);

void bdwrite(struct buf *bp);

void bawrite(struct buf *bp);

void brelse(struct buf *bp);

struct buf *getblk(DeviceCode *dev, int blkno);

void iowait(struct buf *bp);

void notavail(struct buf *bp);

void iodone(struct buf *bp);

void clrbuf(int *bp);

void binit(void);
void devstart(struct buf *bp, int *devloc, intptr_t *devblk, int hbcom);
void rhstart(struct buf *bp, int *devloc, int devblk, int *abae);
void mapfree(struct buf *bp);
void bflush(DeviceCode *dev);

struct buf *incore(DeviceCode *adev, int blkno);
void geterror(struct buf *abp);

#endif // UNIX_BIO_H
