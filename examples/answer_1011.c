#include <stdio.h>
#include <string.h>
typedef unsigned __int128 u128;

u128 parse_u128(const char* s) {
    u128 x = 0;
    for (int i = 0; s[i]; i++) if ('0' <= s[i] && s[i] <= '9') x = x * 10 + (u128)(s[i] - '0');
    return x;
}

u128 add_mod(u128 a, u128 b, u128 mod) {
    a %= mod; b %= mod;
    return a >= mod - b ? a - (mod - b) : a + b;
}

u128 mul_mod(u128 a, u128 b, u128 mod) {
    u128 res = 0;
    a %= mod;
    while (b) {
        if (b & 1) res = add_mod(res, a, mod);
        b >>= 1;
        if (b) a = add_mod(a, a, mod);
    }
    return res;
}

u128 pow_mod(u128 a, u128 e, u128 mod) {
    u128 res = 1;
    while (e) {
        if (e & 1) res = mul_mod(res, a, mod);
        e >>= 1;
        if (e) a = mul_mod(a, a, mod);
    }
    return res;
}

int is_prime_u128(u128 n) {
    if (n < 2) return 0;
    int small[] = {2,3,5,7,11,13,17,19,23,29,31,37};
    for (int i = 0; i < 12; i++) {
        if (n == (u128)small[i]) return 1;
        if (n % (u128)small[i] == 0) return 0;
    }
    u128 d = n - 1;
    int s = 0;
    while ((d & 1) == 0) { d >>= 1; s++; }
    unsigned long long bases[] = {2ULL, 3ULL, 5ULL, 7ULL, 11ULL, 13ULL, 17ULL, 19ULL, 23ULL, 29ULL, 31ULL, 37ULL, 41ULL, 43ULL, 47ULL, 53ULL, 59ULL, 61ULL, 67ULL, 71ULL, 73ULL, 79ULL, 83ULL, 89ULL, 97ULL};
    for (int i = 0; i < 25; i++) {
        u128 a = (u128)bases[i] % n;
        if (a == 0) continue;
        u128 x = pow_mod(a, d, n);
        if (x == 1 || x == n - 1) continue;
        int ok = 0;
        for (int r = 1; r < s; r++) {
            x = mul_mod(x, x, n);
            if (x == n - 1) { ok = 1; break; }
        }
        if (!ok) return 0;
    }
    return 1;
}

int main(void) {
    char s[64];
    scanf("%63s", s);
    u128 n = parse_u128(s);
    printf("%s is %sprime number\n", s, is_prime_u128(n) ? "" : "not ");
    return 0;
}
