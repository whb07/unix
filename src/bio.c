
#include "bio.h"
#include "asmcalls.h"
#include <stdint.h>
#include <string.h>

#include "buf.h"
#include "conf.h"
#include "param.h"
#include "prf.h"
#include "proc.h"
#include "seg.h"
#include "slp.h"
#include "systm.h"
#include "user.h"
/*
 * This is the set of buffers proper, whose heads
 * were declared in buf.h.  There can exist buffer
 * headers not pointing here that are used purely
 * as arguments to the I/O routines to describe
 * I/O to be done-- e.g. swbuf, just below, for
 * swapping.
 */
char buffers[NBUF][514];
struct buf swbuf;

/*
 * Declarations of the tables for the magtape devices;
 * see bdwrite.
 */
int tmtab;
int httab;

/*
 * The following several routines allocate and free
 * buffers with various side effects.  In general the
 * arguments to an allocate routine are a device and
 * a block number, and the value is a pointer to
 * to the buffer header; the buffer is marked "busy"
 * so that no on else can touch it.  If the block was
 * already in core, no I/O need be done; if it is
 * already busy, the process waits until it becomes free.
 * The following routines allocate a buffer:
 *	getblk
 *	bread
 *	breada
 * Eventually the buffer must be released, possibly with the
 * side effect of writing it out, by using one of
 *	bwrite
 *	bdwrite
 *	bawrite
 *	brelse
 */

/*
 * Read in (if necessary) the block and return a buffer pointer.
 */
struct buf *bread(DeviceCode *dev, intptr_t *blkno) {
  struct buf *rbp;

  rbp = getblk(dev, blkno);
  if (rbp->b_flags & B_DONE)
    return (rbp);
  rbp->b_flags |= B_READ;
  rbp->b_wcount = -256;
  (*bdevsw[dev->d_major].d_strategy)(rbp);
  iowait(rbp);
  return (rbp);
}

/*
 * Read in the block, like bread, but also start I/O on the
 * read-ahead block (which is not allocated to the caller)
 */
struct buf *breada(DeviceCode *adev, int blkno, int rablkno) {
  struct buf *rbp, *rabp;
  intptr_t dev;

  dev = (intptr_t)adev;
  rbp = 0;
  if (!incore(dev, blkno)) {
    rbp = getblk(dev, blkno);
    if ((rbp->b_flags & B_DONE) == 0) {
      rbp->b_flags |= B_READ;
      rbp->b_wcount = -256;
      (*bdevsw[adev->d_major].d_strategy)(rbp);
    }
  }
  if (rablkno && !incore(dev, rablkno)) {
    rabp = getblk(dev, rablkno);
    if (rabp->b_flags & B_DONE)
      brelse(rabp);
    else {
      rabp->b_flags |= B_READ | B_ASYNC;
      rabp->b_wcount = -256;
      (*bdevsw[adev->d_major].d_strategy)(rabp);
    }
  }
  if (rbp == 0)
    return (bread(dev, blkno));
  iowait(rbp);
  return (rbp);
}

/*
 * Write the buffer, waiting for completion.
 * Then release the buffer.
 */
void bwrite(struct buf *bp) {
  struct buf *rbp;
  int flag;

  rbp = bp;
  flag = rbp->b_flags;
  rbp->b_flags &= ~(B_READ | B_DONE | B_ERROR | B_DELWRI);
  rbp->b_wcount = -256;
  DeviceCode *device = (DeviceCode *)rbp->b_dev;
  (*bdevsw[device->d_major].d_strategy)(rbp);
  if ((flag & B_ASYNC) == 0) {
    iowait(rbp);
    brelse(rbp);
  } else if ((flag & B_DELWRI) == 0)
    geterror(rbp);
}

/*
 * Release the buffer, marking it so that if it is grabbed
 * for another purpose it will be written out before being
 * given up (e.g. when writing a partial block where it is
 * assumed that another write for the same block will soon follow).
 * This can't be done for magtape, since writes must be done
 * in the same order as requested.
 */
void bdwrite(struct buf *bp) {
  struct buf *rbp;
  struct devtab *dp;

  rbp = bp;
  DeviceCode *dev = (DeviceCode *)rbp->b_dev;
  dp = bdevsw[dev->d_major].d_tab;
  if (dp == &tmtab || dp == &httab)
    bawrite(rbp);
  else {
    rbp->b_flags |= B_DELWRI | B_DONE;
    brelse(rbp);
  }
}

/*
 * Release the buffer, start I/O on it, but don't wait for completion.
 */
void bawrite(struct buf *bp) {
  struct buf *rbp;

  rbp = bp;
  rbp->b_flags |= B_ASYNC;
  bwrite(rbp);
}

/*
 * release the buffer, with no I/O implied.
 */
void brelse(struct buf *bp) {
  struct buf *rbp, **backp;
  int sps;

  rbp = bp;
  if (rbp->b_flags & B_WANTED)
    wakeup(rbp);
  if (bfreelist.b_flags & B_WANTED) {
    bfreelist.b_flags &= ~B_WANTED;
    wakeup(&bfreelist);
  }
  if (rbp->b_flags & B_ERROR) {
    DeviceCode *dev = (DeviceCode *)rbp->b_dev;
    dev->d_minor = -1; /* no assoc. on error */
    backp = &bfreelist.av_back;
    sps = PS;
    //        spl6();
    rbp->b_flags &= ~(B_WANTED | B_BUSY | B_ASYNC);
    (*backp)->av_forw = rbp;
    rbp->av_back = *backp;
    *backp = rbp;
    rbp->av_forw = &bfreelist;
  }
}

/*
 * See if the block is associated with some buffer
 * (mainly to avoid getting hung up on a wait in breada)
 */
struct buf *incore(DeviceCode *adev, int blkno) {
  intptr_t dev;
  struct buf *bp;
  struct devtab *dp;

  dev = (intptr_t)adev;
  dp = (struct devtab *)bdevsw[adev->d_major].d_tab;
  for (bp = dp->b_forw; bp != dp; bp = bp->b_forw)
    if (bp->b_blkno == blkno && bp->b_dev == dev)
      return (bp);
  return (0);
}

/*
 * Assign a buffer for the given block.  If the appropriate
 * block is already associated, return it; otherwise search
 * for the oldest non-busy buffer and reassign it.
 * When a 512-byte area is wanted for some random reason
 * (e.g. during exec, for the user arglist) getblk can be called
 * with device NODEV to avoid unwanted associativity.
 */
struct buf *getblk(DeviceCode *dev, int blkno) {
  struct buf *bp;
  struct devtab *dp;
  extern lbolt;

  if (dev->d_major >= nblkdev)
    panic("blkdev");

loop:
  if (dev < 0)
    dp = &bfreelist;
  else {
    dp = bdevsw[dev->d_major].d_tab;
    if (dp == NULL)
      panic("devtab");
    for (bp = dp->b_forw; bp != dp; bp = bp->b_forw) {
      if (bp->b_blkno != blkno || bp->b_dev != dev)
        continue;
      spl6();
      if (bp->b_flags & B_BUSY) {
        bp->b_flags |= B_WANTED;
        sleep(bp, PRIBIO);
        spl0();
        goto loop;
      }
      spl0();
      notavail(bp);
      return (bp);
    }
  }
  spl6();
  if (bfreelist.av_forw == &bfreelist) {
    bfreelist.b_flags |= B_WANTED;
    sleep(&bfreelist, PRIBIO);
    spl0();
    goto loop;
  }
  spl0();
  notavail(bp = bfreelist.av_forw);
  if (bp->b_flags & B_DELWRI) {
    bp->b_flags |= B_ASYNC;
    bwrite(bp);
    goto loop;
  }
  bp->b_flags = B_BUSY | B_RELOC;
  bp->b_back->b_forw = bp->b_forw;
  bp->b_forw->b_back = bp->b_back;
  bp->b_forw = dp->b_forw;
  bp->b_back = dp;
  dp->b_forw->b_back = bp;
  dp->b_forw = bp;
  bp->b_dev = dev;
  bp->b_blkno = blkno;
  return (bp);
}

/*
 * Wait for I/O completion on the buffer; return errors
 * to the user.
 */
void iowait(struct buf *bp) {
  struct buf *rbp;
  rbp = bp;
  spl6();
  while ((rbp->b_flags & B_DONE) == 0)
    sleep(rbp, PRIBIO);
  spl0();
  geterror(rbp);
}

/*
 * Unlink a buffer from the available list and mark it busy.
 * (internal interface)
 */
void notavail(struct buf *bp) {
  struct buf *rbp;
  int sps;

  rbp = bp;
  sps = (int)PS;
  spl6();
  rbp->av_back->av_forw = rbp->av_forw;
  rbp->av_forw->av_back = rbp->av_back;
  rbp->b_flags |= B_BUSY;
  //    PS->integ = sps;
}

/*
 * Mark I/O complete on a buffer, release it if I/O is asynchronous,
 * and wake up anyone waiting for it.
 */
void iodone(struct buf *bp) {
  struct buf *rbp;
  rbp = bp;
  if (rbp->b_flags & B_MAP) {
    mapfree(rbp);
  }

  rbp->b_flags |= B_DONE;

  if (rbp->b_flags & B_ASYNC) {
    brelse(rbp);
  } else {
    rbp->b_flags &= ~B_WANTED;
    wakeup(rbp);
  }
}

/*
 * Zero the core associated with a buffer.
 */
void clrbuf(int *bp) { memset(bp, 0, 256); }

/*
 * Initialize the buffer I/O system by freeing
 * all buffers and setting all device buffer lists to empty.
 */
void binit(void) {
  struct buf *bp;
  struct devtab *dp;
  int i;
  struct bdevsw *bdp;

  bfreelist.b_forw = bfreelist.b_back = bfreelist.av_forw = bfreelist.av_back =
      &bfreelist;
  for (i = 0; i < NBUF; i++) {
    bp = &buf[i];
    bp->b_dev = -1;
    bp->b_addr = buffers[i];
    bp->b_back = &bfreelist;
    bp->b_forw = bfreelist.b_forw;
    bfreelist.b_forw->b_back = bp;
    bfreelist.b_forw = bp;
    bp->b_flags = B_BUSY;
    brelse(bp);
  }
  i = 0;
  for (bdp = bdevsw; bdp->d_open; bdp++) {
    dp = bdp->d_tab;
    if (dp) {
      dp->b_forw = dp;
      dp->b_back = dp;
    }
    i++;
  }
  nblkdev = i;
}

/*
 * Device start routine for disks
 * and other devices that have the
 * layout of the older DEC controllers (RF, RK, RP, TM)
 */
#define IENABLE 0100
#define WCOM 02
#define RCOM 04
#define GO 01
void devstart(struct buf *bp, int *devloc, intptr_t *devblk, int hbcom) {
  intptr_t *dp;
  struct buf *rbp;
  intptr_t com;

  dp = devloc;
  rbp = bp;
  *dp = devblk;          /* block address */
  *--dp = rbp->b_addr;   /* buffer address */
  *--dp = rbp->b_wcount; /* word count */
  com = (hbcom << 8) | IENABLE | GO | (((intptr_t)(rbp->b_xmem) & 03) << 4);

  if (rbp->b_flags & B_READ) { /* command + x-mem */
    com |= RCOM;
  } else {
    com |= WCOM;
  }
  *--dp = com;
}

/*
 * startup routine for RH controllers.
 */
#define RHWCOM 060
#define RHRCOM 070

void rhstart(struct buf *bp, int *devloc, int devblk, int *abae) {
  intptr_t *dp;
  struct buf *rbp;
  intptr_t com;

  dp = devloc;
  rbp = bp;
  if (cputype == 70)
    *abae = rbp->b_xmem;
  *dp = devblk;          /* block address */
  *--dp = rbp->b_addr;   /* buffer address */
  *--dp = rbp->b_wcount; /* word count */
  com = IENABLE | GO | (((intptr_t)rbp->b_xmem & 03) << 8);
  if (rbp->b_flags & B_READ) /* command + x-mem */
    com |= RHRCOM;
  else
    com |= RHWCOM;
  *--dp = com;
}

///*
// * 11/70 routine to allocate the
// * UNIBUS map and initialize for
// * a unibus device.
// * The code here and in
// * rhstart assumes that an rh on an 11/70
// * is an rh70 and contains 22 bit addressing.
// */
int maplock;
// void mapalloc(struct buf *abp)
//{
//     int i, a;
//     struct buf *bp;
//     struct addresses addr;
//     int *r;
//
//    if(cputype != 70)
//        return;
////    spl6();
//    while(maplock&B_BUSY) {
//        maplock |= B_WANTED;
//        sleep(&maplock, PSWP);
//    }
//    maplock |= B_BUSY;
////    spl0();
//    bp = abp;
//    bp->b_flags |= B_MAP;
//    a = bp->b_xmem;
//    for(i=16; i<32; i=+2)
//        UBMAP-> r[i+1] = a;
//    for(a++; i<48; i=+2)
//        UBMAP->r[i+1] = a;
//    bp->b_xmem = 1;
//}

void mapfree(struct buf *bp) {
  bp->b_flags &= ~B_MAP;
  if (maplock & B_WANTED)
    wakeup(&maplock);
  maplock = 0;
}
//
///*
// * swap I/O
// */
// swap(blkno, coreaddr, count, rdflg
//)
//{
// int *fp;
//
// fp = &swbuf.b_flags;
// spl6();
// while (*fp&B_BUSY) {
//*fp |= B_WANTED;
// sleep(fp, PSWP);
//}
//*
// fp = B_BUSY | B_PHYS | rdflg;
// swbuf.
// b_dev = swapdev;
// swbuf.
// b_wcount = -(count << 5);    /* 32 w/block */
// swbuf.
// b_blkno = blkno;
// swbuf.
// b_addr = coreaddr << 6;    /* 64 b/block */
// swbuf.
// b_xmem = (coreaddr >> 10) & 077;
//(*bdevsw[swapdev>>8].d_strategy)(&swbuf);
// spl6();
// while((*fp&B_DONE)==0)
// sleep(fp, PSWP);
// if (*fp&B_WANTED)
// wakeup(fp);
// spl0();
//*fp &= ~(B_BUSY|B_WANTED);
// return(*fp&B_ERROR);
//}

/*
 * make sure all write-behind blocks
 * on dev (or NODEV for all)
 * are flushed out.
 * (from umount and update)
 */
void bflush(DeviceCode *dev) {
  struct buf *bp;

loop:
  spl6();
  for (bp = bfreelist.av_forw; bp != &bfreelist; bp = bp->av_forw) {
    if (bp->b_flags & B_DELWRI && (dev == NODEV || dev == bp->b_dev)) {
      bp->b_flags |= B_ASYNC;
      notavail(bp);
      bwrite(bp);
      goto loop;
    }
  }
  spl0();
}

/*
 * Raw I/O. The arguments are
 *	The strategy routine for the device
 *	A buffer, which will always be a special buffer
 *	  header owned exclusively by the device for this purpose
 *	The device number
 *	Read/write flag
 * Essentially all the work is computing physical addresses and
 * validating them.
 */
// physio(strat, abp, dev, rw
//)
// struct buf *abp;
// int (*strat)();
//{
// struct buf *bp;
// char *base;
// int nb;
// int ts;
//
// bp = abp;
// base = u.u_base;
///*
// * Check odd base, odd count, and address wraparound
// */
// if (base&01 || u.u_count&01 || base>=base+u.u_count)
// goto
// bad;
// ts = (u.u_tsize + 127) & ~0177;
// if (u.u_sep)
// ts = 0;
// nb = (base >> 6) & 01777;
///*
// * Check overlap with text. (ts and nb now
// * in 64-byte clicks)
// */
// if (nb < ts)
// goto
// bad;
///*
// * Check that transfer is either entirely in the
// * data or in the stack: that is, either
// * the end is in the data or the start is in the stack
// * (remember wraparound was already checked).
// */
// if ((((base+u.u_count)>>6)&01777) >= ts+u.u_dsize
//&& nb < 1024-u.u_ssize)
// goto
// bad;
// spl6();
// while (bp->b_flags&B_BUSY) {
// bp->b_flags |= B_WANTED;
// sleep(bp, PRIBIO);
//}
// bp->
// b_flags = B_BUSY | B_PHYS | rw;
// bp->
// b_dev = dev;
///*
// * Compute physical address by simulating
// * the segmentation hardware.
// */
// bp->
// b_addr = base & 077;
// base = (u.u_sep ? UDSA : UISA)->r[nb >> 7] + (nb & 0177);
// bp->
// b_addr = +base << 6;
// bp->
// b_xmem = (base >> 10) & 077;
// bp->
// b_blkno = lshift(u.u_offset, -9);
// bp->
// b_wcount = -((u.u_count >> 1) & 077777);
// bp->
// b_error = 0;
// u.u_procp->p_flag |= SLOCK;
//(*strat)(bp);
// spl6();
// while ((bp->b_flags&B_DONE) == 0)
// sleep(bp, PRIBIO);
// u.u_procp->p_flag &= ~SLOCK;
// if (bp->b_flags&B_WANTED)
// wakeup(bp);
// spl0();
// bp->b_flags &= ~(B_BUSY|B_WANTED);
// u.
// u_count = (-bp->b_resid) << 1;
// geterror(bp);
// return;
// bad:
// u.
// u_error = EFAULT;
//}
//
/*
 * Pick up the device's error number and pass it to the user;
 * if there is an error but the number is 0 set a generalized
 * code.  Actually the latter is always true because devices
 * don't yet return specific errors.
 */
void geterror(struct buf *abp) {
  struct buf *bp;

  bp = abp;
  if (bp->b_flags & B_ERROR)
    if ((u.u_error = bp->b_error) == 0)
      u.u_error = EIO;
}
