#include <stdio.h>
int main(void) {
    int n;
    long long dp[26] = {0};
    scanf("%d", &n);
    dp[0] = 1;
    dp[1] = 1;
    for (int i = 2; i <= n; i++) dp[i] = dp[i - 1] + dp[i - 2];
    printf("%lld\n", dp[n]);
    return 0;
}
