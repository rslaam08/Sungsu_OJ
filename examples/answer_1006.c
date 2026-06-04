#include <stdio.h>
int n, a[8], vals[4] = {0, 4, 7, 9};
int ans = 0;
void dfs(int pos, int rem, int sum) {
    if (pos == rem) {
        if (sum >= 40) ans++;
        return;
    }
    for (int i = 0; i < 4; i++) dfs(pos + 1, rem, sum + vals[i]);
}
int main(void) {
    int sum = 0;
    scanf("%d", &n);
    for (int i = 0; i < n; i++) { scanf("%d", &a[i]); sum += a[i]; }
    dfs(0, 8 - n, sum);
    printf("%d\n", ans);
    return 0;
}
