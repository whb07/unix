#include <stdio.h>
#include <stdlib.h>


//int hello(struct map *mp){
//    register struct map *bp;
//    bp = mp;
//    for(char *c = bp->m_addr; *c != '\0'; c++){
//        printf("%c\n", *c);
//    }
//    return 0;
//}
struct	Person
        {
int age;
        } Person[100];




int main() {
    char *str;
    struct Person p = Person[75];
    for(int i = 0; i < 10; i++){
        printf("%d\n", Person[i].age);
    }

    /* Initial memory allocation */
    str = (char *) malloc(256);

    const char *example = "Hello World";

    snprintf(str, 256, "%s", example);
    free(str);
    return 0;
}
