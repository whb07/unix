//
// Created by wbright on 8/21/21.
//

#ifndef UNIX_BIO_H
#define UNIX_BIO_H
#include "buf.h"
#include "conf.h"


void bwrite(struct buf *bp);

void bdwrite(struct buf *bp);

void bawrite(struct buf *bp);

struct buf* getblk(DeviceCode *dev, int blkno);

struct buf* bread(DeviceCode *dev, int blkno);
struct buf * breada(DeviceCode *adev, int blkno, int rablkno);
void brelse(struct buf *bp);
void clrbuf(int *bp);
#endif //UNIX_BIO_H
