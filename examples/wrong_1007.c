#include <stdio.h>
int main(void) {
    long long k, sum = 0, x;
    int n, a, cnt = 0;
    scanf("%lld %d %d", &k, &n, &a);
    for (int i = 0; i < n; i++) { scanf("%lld", &x); if (sum + x <= k) { sum += x; cnt++; } }
    printf("%d\n", cnt);
    return 0;
}
