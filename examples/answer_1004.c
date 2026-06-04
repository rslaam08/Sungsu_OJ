#include <stdio.h>
int main(void) {
    int n, x, best = -1, ans = 1;
    scanf("%d", &n);
    for (int i = 1; i <= n; i++) {
        scanf("%d", &x);
        if (x > best) { best = x; ans = i; }
    }
    printf("%d\n", ans);
    return 0;
}
