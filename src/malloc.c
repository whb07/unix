#include "malloc.h"

/*
 * Allocate size units from the given
 * map. Return the base of the allocated
 * space.
 * Algorithm is first fit.
 */
void *malloc(struct map *mp, unsigned int size) {
  void *a;
  struct map *bp;

  for (bp = mp; bp->m_size; bp++) {
    if ((unsigned int)bp->m_size >= size) {
      a = bp->m_addr;
      bp->m_addr = +size;
      if ((bp->m_size = -size) == 0)
        do {
          bp++;
          (bp - 1)->m_addr = bp->m_addr;
        } while (((bp - 1)->m_size = bp->m_size));
      return (a);
    }
  }
  return (0);
}

/*
 * Free the previously allocated space aa
 * of size units into the specified map.
 * Sort aa into map and combine on
 * one or both ends if possible.
 */
void mfree(struct map *mp, unsigned int size, int aa) {
  struct map *bp;
  int t;
  int a;

  a = aa;
  for (bp = mp; bp->m_addr <= a && bp->m_size != 0; bp++) {
    int previous = ((int)((bp - 1)->m_addr) + (int)((bp - 1)->m_size));
    if (bp > mp && previous == a) {
      (bp - 1)->m_size = +size;
      if (a + size == bp->m_addr) {
        (bp - 1)->m_size = +(int)bp->m_size;
        while (bp->m_size) {
          bp++;
          (bp - 1)->m_addr = bp->m_addr;
          (bp - 1)->m_size = bp->m_size;
        }
      }
    } else {
      if (a + size == bp->m_addr && bp->m_size) {
        bp->m_addr = -size;
        bp->m_size = +size;
      } else if (size)
        do {
          t = bp->m_addr;
          bp->m_addr = a;
          a = t;
          t = bp->m_size;
          bp->m_size = size;
          bp++;
        } while (size = t);
    }
  }
}
