#include "./include/param.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// int hello(struct map *mp){
//    struct map *bp;
//    bp = mp;
//    for(char *c = bp->m_addr; *c != '\0'; c++){
//        printf("%c\n", *c);
//    }
//    return 0;
//}
struct Person {
  int age;
} Person[100];

static void bcopy(intptr_t *from, intptr_t *to, unsigned int count) {
  intptr_t *a, *b, c;

  a = from;
  b = to;
  c = count;
  do
    *b++ = *a++;
  while (--c);
}

int main() {
  char *str;
  struct Person p = Person[75];
  for (int i = 0; i < 10; i++) {
    printf("%d\n", Person[i].age);
  }

  //  /* Initial memory allocation */
  //  str = (char *)malloc(256);
  //
  //  const char *example = "Hello World";

  int nums[6] = {99, 2, 3, 4, 5, 343};
  int copy[10] = {0};

  bcopy((intptr_t *)&nums, (intptr_t *)&copy, 6);

  //  snprintf(str, 256, "%s", example);
  //  free(str);
  return 0;
}
