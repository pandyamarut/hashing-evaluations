#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <openssl/sha.h>

#define MESSAGE_SIZE 1024*1024   // Size of input message in bytes
#define NUM_ITERATIONS 10000     // Number of iterations to run

int main(int argc, char *argv[]) {
    uint8_t *message = malloc(MESSAGE_SIZE);
    uint8_t digest[SHA256_DIGEST_LENGTH];
    struct timeval start_time, end_time;
    double total_time;

    // Generate random input message
    srand(time(NULL));
    for (int i = 0; i < MESSAGE_SIZE; i++) {
        message[i] = rand() % 256;
    }

    // Measure SHA-256 throughput
    gettimeofday(&start_time, NULL);
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        SHA256(message, MESSAGE_SIZE, digest);
    }
    gettimeofday(&end_time, NULL);

    // Calculate total time and throughput
    total_time = ((end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
                  (end_time.tv_usec - start_time.tv_usec)) / 1000000.0;
    double throughput = ((double) MESSAGE_SIZE * NUM_ITERATIONS) / total_time;

    // Print results
    printf("SHA-256 throughput: %.2f MB/s\n", throughput / (1024*1024));
    printf("Total time: %.2f s\n", total_time);

    free(message);
    return 0;
}
