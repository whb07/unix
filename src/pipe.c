//
// Created by wbright on 8/23/21.
//

#include "pipe.h"

#include "alloc.h"
#include "param.h"
#include "systm.h"
#include "user.h"
#include "inode.h"
#include "fio.h"
#include "iget.h"
#include "file.h"
#include "slp.h"
#include "sig.h"
#include "reg.h"
#include "rdwri.h"

/*
 * Max allowable buffering per pipe.
 * This is also the max size of the
 * file created to implement the pipe.
 * If this size is bigger than 4096,
 * pipes will be implemented in LARG
 * files, which is probably not good.
 */
#define	PIPSIZ	4096

/*
 * The sys-pipe entry.
 * Allocate an inode on the root device.
 * Allocate 2 file structures.
 * Put it all together with flags.
 */
void pipe(void)
{
  struct inode *ip;
  struct file *rf, *wf;
  int r;

  ip = ialloc(rootdev);
  if(ip == NULL)
    return;
  rf = falloc();
  if(rf == NULL) {
    iput(ip);
    return;
  }
  r = u.u_ar0[R0];
  wf = falloc();
  if(wf == NULL) {
    rf->f_count = 0;
    u.u_ofile[r] = NULL;
    iput(ip);
    return;
  }
  u.u_ar0[R1] = u.u_ar0[R0];
  u.u_ar0[R0] = r;
  wf->f_flag = FWRITE|FPIPE;
  wf->f_inode = ip;
  rf->f_flag = FREAD|FPIPE;
  rf->f_inode = ip;
  ip->i_count = 2;
  ip->i_flag = IACC|IUPD;
  ip->i_mode = IALLOC;
}

/*
 * Read call directed to a pipe.
 */
void readp(int *fp)
{
  struct file *rp;
  struct inode *ip;

  rp = (struct file *) fp;
  ip = rp->f_inode;

  loop:
  /*
   * Very conservative locking.
   */

  plock(ip);

  /*
   * If the head (read) has caught up with
   * the tail (write), reset both to 0.
   */

  if(rp->f_offset[1] == ip->i_size1) {
    if(rp->f_offset[1] != 0) {
      rp->f_offset[1] = 0;
      ip->i_size1 = 0;
      if(ip->i_mode&IWRITE) {
        ip->i_mode &= ~IWRITE;
        wakeup(ip+1);
      }
    }

    /*
     * If there are not both reader and
     * writer active, return without
     * satisfying read.
     */

    prele(ip);
    if(ip->i_count < 2)
      return;
    ip->i_mode |= IREAD;
    sleep(ip+2, PPIPE);
    goto loop;
  }

  /*
   * Read and return
   */

  u.u_offset[0] = 0;
  u.u_offset[1] = rp->f_offset[1];
  readi(ip);
  rp->f_offset[1] = u.u_offset[1];
  prele(ip);
}


/*
 * Write call directed to a pipe.
 */
void writep(int *fp)
{
  struct file *rp;
  struct inode *ip;
   char * c;

  rp = (struct file *) fp;
  ip = (struct inode *) rp->f_inode;
  c = u.u_count;

  loop:

  /*
   * If all done, return.
   */

  plock((int *) ip);
  if(c == 0) {
    prele((int *) ip);
    u.u_count = 0;
    return;
  }

  /*
   * If there are not both read and
   * write sides of the pipe active,
   * return error and signal too.
   */

  if(ip->i_count < 2) {
    prele((int *) ip);
    u.u_error = EPIPE;
    psignal(u.u_procp, SIGPIPE);
    return;
  }

  /*
   * If the pipe is full,
   * wait for reads to deplete
   * and truncate it.
   */

  if(ip->i_size1 == PIPSIZ) {
    ip->i_mode |= IWRITE;
    prele(ip);
    sleep(ip+1, PPIPE);
    goto loop;
  }

  /*
   * Write what is possible and
   * loop back.
   */

  u.u_offset[0] = 0;
  u.u_offset[1] = ip->i_size1;
  u.u_count = min(c, PIPSIZ - (int) u.u_offset[1]);
  c -= u.u_count;
  writei(ip);
  prele(ip);
  if(ip->i_mode&IREAD) {
    ip->i_mode &= ~IREAD;
    wakeup(ip+2);
  }
  goto loop;
}

/*
 * Lock a pipe.
 * If its already locked,
 * set the WANT bit and sleep.
 */
void plock(int *ip)
{
  struct inode *rp;

  rp = (struct inode *) ip;
  while(rp->i_flag&ILOCK) {
    rp->i_flag |= IWANT;
    sleep((intptr_t *) rp, PPIPE);
  }
  rp->i_flag |= ILOCK;
}

/*
 * Unlock a pipe.
 * If WANT bit is on,
 * wakeup.
 * This routine is also used
 * to unlock inodes in general.
 */
void prele(int *ip)
{
  struct inode *rp;

  rp = (struct inode *) ip;
  rp->i_flag &= ~ILOCK;
  if(rp->i_flag&IWANT) {
    rp->i_flag &= ~IWANT;
    wakeup(rp);
  }
}
