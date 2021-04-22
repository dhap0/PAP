#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <sys/time.h>
double getusec_() {
        struct timeval time;
        gettimeofday(&time, NULL);
        return ((double)time.tv_sec * (double)1e6 + (double)time.tv_usec);
}

#define START_COUNT_TIME stamp = getusec_();
#define STOP_COUNT_TIME stamp = getusec_() - stamp;\
                        stamp = stamp/1e6;

double result=0.0, result_even=0.0, result_odd=0.0;

double foo() {
#pragma omp parallel
    {
    for (long i = 0; i < 1000000; i++) {
        if (i%2) {
            #pragma omp critical(even)
            result_even += sqrt(i);
            }
        else {
            #pragma omp critical(odd)
            result_odd += sqrt(i);
            }
    }

    #pragma omp barrier

    #pragma omp critical
    result += (result_even + result_odd);
    }
    return(result);
}

int main(int argc, char *argv[]) {
    double stamp;
    START_COUNT_TIME;
    double total = foo();
    STOP_COUNT_TIME;
    printf ("Execution time for goo(10): %0.6f with total=%0.0f\n", stamp, total);
}
