#include <stdio.h>
#include <string.h>

int main(void) {
    int x;
    scanf("%d", &x);
    char vla[x];
    (void)vla;
    return 0;
}
