#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "blake3.h"
#include <pthread.h>

#define MESSAGE_SIZE 1024*1024   // Size of input message in bytes
#define NUM_ITERATIONS 10000     // Number of iterations to run
#define NUM_THREADS 4            // Number of threads to use

struct thread_args {
    uint8_t *message;
    int num_iterations;
};

void *hash_thread(void *arg) {
    struct thread_args *args = (struct thread_args *) arg;
    uint8_t digest[BLAKE3_OUT_LEN];
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    for (int i = 0; i < args->num_iterations; i++) {
        blake3_hasher_update(&hasher, args->message, MESSAGE_SIZE);
    }
    blake3_hasher_finalize(&hasher, digest, BLAKE3_OUT_LEN);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    uint8_t *message = malloc(MESSAGE_SIZE);
    pthread_t threads[NUM_THREADS];
    struct thread_args args[NUM_THREADS];
    struct timeval start_time, end_time;
    double total_time;

    // Generate random input message
    srand(time(NULL));
    for (int i = 0; i < MESSAGE_SIZE; i++) {
        message[i] = rand() % 256;
    }

    // Divide iterations evenly among threads
    int iterations_per_thread = NUM_ITERATIONS / NUM_THREADS;
    int remaining_iterations = NUM_ITERATIONS % NUM_THREADS;

    // Measure BLAKE3 hash rate and memory usage
    struct rusage usage_start, usage_end;
    getrusage(RUSAGE_SELF, &usage_start);
    gettimeofday(&start_time, NULL);
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].message = message;
        args[i].num_iterations = iterations_per_thread;
        if (i == NUM_THREADS - 1) {
            // Add remaining iterations to last thread
            args[i].num_iterations += remaining_iterations;
        }
        pthread_create(&threads[i], NULL, hash_thread, (void *)&args[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    gettimeofday(&end_time, NULL);
    getrusage(RUSAGE_SELF, &usage_end);

    // Calculate total time and hash rate
    total_time = ((end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
                  (end_time.tv_usec - start_time.tv_usec)) / 1000000.0;
    double hash_rate = NUM_ITERATIONS / total_time;

    // Print results
    printf("BLAKE3 hash rate: %.2f hashes/s\n", hash_rate);
    printf("Total time: %.2f s\n", total_time);
    printf("Maximum resident set size: %ld KB\n", usage_end.ru_maxrss - usage_start.ru_maxrss);

    free(message);
    return 0;
}
