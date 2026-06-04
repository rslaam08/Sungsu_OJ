#include <stdio.h>
#include <string.h>
char s[10005], ans[10005];
int main(void) {
    scanf("%10000s", s);
    int n = (int)strlen(s), flip = 0, idx = 0;
    for (int i = n - 1; i >= 0; i--) {
        int bit = (s[i] - '0') ^ flip;
        if (bit == 0) ans[idx++] = '0';
        else { ans[idx++] = '1'; flip ^= 1; }
    }
    for (int i = idx - 1; i >= 0; i--) putchar(ans[i]);
    putchar('\n');
    return 0;
}
