#include <stdio.h>
#include <stdlib.h>
long long x[10005], prefix[10005];
int cmp_ll(const void* a, const void* b) {
    long long A = *(const long long*)a, B = *(const long long*)b;
    return (A > B) - (A < B);
}
int main(void) {
    long long k;
    int n, a;
    scanf("%lld %d %d", &k, &n, &a);
    for (int i = 1; i <= n; i++) scanf("%lld", &x[i]);
    qsort(x + 1, n, sizeof(long long), cmp_ll);
    for (int i = 1; i <= n; i++) prefix[i] = prefix[i - 1] + x[i];
    int ans = 0;
    for (int m = 1; m <= n; m++) {
        int d = a < m ? a : m;
        long long doubled_cost = 2 * prefix[m] - (prefix[m] - prefix[m - d]);
        if (doubled_cost <= 2 * k) ans = m;
    }
    printf("%d\n", ans);
    return 0;
}
