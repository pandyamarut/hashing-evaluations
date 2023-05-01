#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include "blake3.h"

#define MESSAGE_SIZE 1024*1024   // Size of input message in bytes
#define NUM_ITERATIONS 10000     // Number of iterations to run

int main(int argc, char *argv[]) {
    uint8_t *message = malloc(MESSAGE_SIZE);
    uint8_t digest[BLAKE3_OUT_LEN];
    struct timeval start_time, end_time;
    double total_time;

    // Generate random input message
    srand(time(NULL));
    for (int i = 0; i < MESSAGE_SIZE; i++) {
        message[i] = rand() % 256;
    }

    // Measure BLAKE3 hash rate
    gettimeofday(&start_time, NULL);
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        blake3_hasher_update(&hasher, message, MESSAGE_SIZE);
    }
    blake3_hasher_finalize(&hasher, digest, BLAKE3_OUT_LEN);
    gettimeofday(&end_time, NULL);

    // Calculate total time and hash rate
    total_time = ((end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
                  (end_time.tv_usec - start_time.tv_usec)) / 1000000.0;
    double hash_rate = NUM_ITERATIONS / total_time;

    // Print results
    printf("BLAKE3 hash rate: %.2f hashes/s\n", hash_rate);
    printf("Total time: %.2f s\n", total_time);

    free(message);
    return 0;
}
