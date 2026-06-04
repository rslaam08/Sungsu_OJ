#include <stdio.h>
#include <math.h>
int main(void) {
    int L;
    scanf("%d", &L);
    double pi = 3.14;
    double T = 2.0 * pi * sqrt((double)L / 9.81);
    printf("%.1f\n", T);
    return 0;
}
