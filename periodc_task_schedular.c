#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

struct timespec start;

double get_elapsed_time()
{
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    double elapsed = (current_time.tv_sec - start.tv_sec) + (current_time.tv_nsec - start.tv_nsec) / 1e9;
    return elapsed;
}

void timer_handler(int signum)
{
    static int count = 0;
    printf("\n---- Timer count %d ----\n", ++count);

    printf("[%.6f s] Task A executed\n", get_elapsed_time());
    if (count % 2 == 0)
    {
        printf("[%.6f s] Task B executed\n", get_elapsed_time());
    }
}

int main()
{
    struct sigaction sa;
    struct itimerval timer;

    clock_gettime(CLOCK_MONOTONIC, &start);

    sa.sa_handler = &timer_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);

    timer.it_value.tv_sec = 1;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 2;
    timer.it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, &timer, NULL);

    printf("Timer started. PID: %d\n", getpid());

    while (1)
    {
        pause();
    }

    return 0;
}
