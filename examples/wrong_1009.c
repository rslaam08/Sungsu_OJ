#include <stdio.h>
int main(void) {
    int n; scanf("%d", &n);
    for (int i = 0; i < n; i++) {
        if (i) putchar(' ');
        printf("%d", i + 2);
    }
    putchar('\n');
    return 0;
}
