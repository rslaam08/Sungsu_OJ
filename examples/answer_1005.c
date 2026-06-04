#include <stdio.h>
#include <math.h>
int main(void) {
    int L;
    scanf("%d", &L);
    double pi = acos(-1.0);
    double T = 2.0 * pi * sqrt(100.0 * L / 9.81);
    printf("%.1f\n", T);
    return 0;
}
