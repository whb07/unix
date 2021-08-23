//
// Created by wbright on 8/22/21.
//

#ifndef UNIX_SRC_MATHS_H_
#define UNIX_SRC_MATHS_H_

static long ldiv(long int numer, long int denom) { return numer / denom; }

static long lrem(long int numer, long int denom) { return numer % denom; }

static long int lshift(int I, int SHIFT) {
  return I << SHIFT;
}

static int dpcmp(int a, int b, int c, int d) {
  return 0;
}
#endif // UNIX_SRC_MATHS_H_
