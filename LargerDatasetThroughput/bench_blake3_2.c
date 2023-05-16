#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include "blake3.h"
#include <pthread.h>

#define MESSAGE_SIZE 32           // Size of each input message chunk in bytes
#define NUM_ITERATIONS 500000     // Number of iterations to run
#define NUM_THREADS 4              // Number of threads to use

struct thread_args {
    uint8_t *message;
    int num_iterations;
};

void *hash_thread(void *arg) {
    struct thread_args *args = (struct thread_args *)arg;
    uint8_t digest[BLAKE3_OUT_LEN];
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    for (int i = 0; i < args->num_iterations; i++) {
        blake3_hasher_update(&hasher, args->message + (i * MESSAGE_SIZE), MESSAGE_SIZE);
    }
    blake3_hasher_finalize(&hasher, digest, BLAKE3_OUT_LEN);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    uint8_t **message = malloc(NUM_ITERATIONS * sizeof(uint8_t *));
    pthread_t threads[NUM_THREADS];
    struct thread_args args[NUM_THREADS];
    struct timeval start_time, end_time;
    double total_time;

    // Open the input file
    FILE *input_file = fopen("input.txt", "rb");
    if (input_file == NULL) {
        printf("Failed to open the input file.\n");
        return 1;
    }

    // Read the input data in chunks
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        message[i] = malloc(MESSAGE_SIZE);
        size_t bytes_read = fread(message[i], 1, MESSAGE_SIZE, input_file);
        if (bytes_read != MESSAGE_SIZE) {
            printf("Failed to read the input file.\n");
            return 1;
        }
    }

    // Close the input file
    fclose(input_file);

    // Divide iterations evenly among threads
    int iterations_per_thread = NUM_ITERATIONS / NUM_THREADS;
    int remaining_iterations = NUM_ITERATIONS % NUM_THREADS;

    // Measure BLAKE3 hash rate
    gettimeofday(&start_time, NULL);
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].message = *message;
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

    // Calculate total time and hash rate
    total_time = ((end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
                  (end_time.tv_usec - start_time.tv_usec)) / 1000000.0;
    double hash_rate = NUM_ITERATIONS / total_time;

    // Print results
    printf("BLAKE3 hash rate: %.2f hashes/s\n", hash_rate);
    printf("Total time: %.2f s\n", total_time);

    // Free the memory allocated for the chunks
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        free(message[i]);
    }
    free(message);

    return 0;
}
