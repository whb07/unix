//
// Created by wbright on 8/22/21.
//

#ifndef UNIX_MALLOC_H
#define UNIX_MALLOC_H

/*
 * Structure of the coremap and swapmap
 * arrays. Consists of non-zero count
 * and base address of that many
 * contiguous units.
 * (The coremap unit is 64 bytes,
 * the swapmap unit is 512 bytes)
 * The addresses are increasing and
 * the list is terminated with the
 * first zero count.
 */
struct map {
  char *m_size;
  char *m_addr;
};

void *malloc(struct map *mp, unsigned int size);
void mfree(struct map *mp, unsigned int size, int aa);

#endif // UNIX_MALLOC_H
