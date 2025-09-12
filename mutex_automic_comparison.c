#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

static long atomic_counter = 0;   
static long mutex_counter = 0; 
static pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    long iterations;
} bench_arg_t;

// Worker: Lock-free atomic increment
static void* worker_atomic(void* arg) {
    bench_arg_t* argument = (bench_arg_t*) arg;
    long iterations = argument->iterations;

    for (long i = 0; i < iterations; i++) {
        __atomic_fetch_add(&atomic_counter, 1, __ATOMIC_RELAXED);
    }
    return NULL;
}

// Worker: Mutex-protected increment
static void* worker_mutex(void* arg) {
    bench_arg_t* argument = (bench_arg_t*) arg;
    long iterations = argument->iterations;

    for (long i = 0; i < iterations; i++) {
        pthread_mutex_lock(&counter_mutex);
        ++mutex_counter;
        pthread_mutex_unlock(&counter_mutex);
    }
    return NULL;
}

// Get current time in seconds
static double convert_to_seconds(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

int main(int argc, char** argv) {
    int number_of_threads = (argc > 1) ? atoi(argv[1]) : 8;      //8 threads
    long iterations       = (argc > 2) ? atol(argv[2]) : 2000000; //2M iterations per thread

    pthread_t* threads = malloc(sizeof(pthread_t) * number_of_threads);
    bench_arg_t arg = { iterations };

    printf("Benchmarking with %d threads × %ld iterations each (total = %ld ops)\n\n",
           number_of_threads, iterations, number_of_threads * iterations);

    
    atomic_counter = 0;
    double start_atomic = convert_to_seconds();

    for (int i = 0; i < number_of_threads; i++) {
        pthread_create(&threads[i], NULL, worker_atomic, &arg);
    }
    for (int i = 0; i < number_of_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    double end_atomic = convert_to_seconds();
    printf("[Atomic] counter = %ld | time = %.6f sec\n",
           atomic_counter, end_atomic - start_atomic);

    
    mutex_counter = 0;
    double start_mutex = convert_to_seconds();

    for (int i = 0; i < number_of_threads; i++) {
        pthread_create(&threads[i], NULL, worker_mutex, &arg);
    }
    for (int i = 0; i < number_of_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    double end_mutex = convert_to_seconds();
    printf("[Mutex ] counter = %ld | time = %.6f sec\n",
           mutex_counter, end_mutex - start_mutex);

    free(threads);
    return 0;
}