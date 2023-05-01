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
    uint8_t digest[SHA512_DIGEST_LENGTH];
    struct timeval start_time, end_time;
    double total_time;

    // Generate random input message
    srand(time(NULL));
    for (int i = 0; i < MESSAGE_SIZE; i++) {
        message[i] = rand() % 256;
    }

    // Measure SHA-512 hashes per second
    gettimeofday(&start_time, NULL);
    int num_hashes = 0;
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        SHA512(message, MESSAGE_SIZE, digest);
        num_hashes++;
    }
    gettimeofday(&end_time, NULL);

    // Calculate total time and hashes per second
    total_time = ((end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
                  (end_time.tv_usec - start_time.tv_usec)) / 1000000.0;
    double hashes_per_second = num_hashes / total_time;

    // Print results
    printf("SHA-512 hashes per second: %.2f\n", hashes_per_second);
    printf("Total time: %.2f s\n", total_time);

    free(message);
    return 0;
}
