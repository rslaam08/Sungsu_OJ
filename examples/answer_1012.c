#include <stdio.h>
#include <stdlib.h>
int lower_bound_int(int* arr, int len, int x) {
    int l = 0, r = len;
    while (l < r) {
        int m = (l + r) / 2;
        if (arr[m] < x) l = m + 1;
        else r = m;
    }
    return l;
}
int main(void) {
    int n, x, len = 0;
    scanf("%d", &n);
    int* dp = (int*)malloc(sizeof(int) * n);
    if (!dp) return 1;
    for (int i = 0; i < n; i++) {
        scanf("%d", &x);
        int pos = lower_bound_int(dp, len, x);
        dp[pos] = x;
        if (pos == len) len++;
    }
    printf("%d\n", len);
    free(dp);
    return 0;
}
