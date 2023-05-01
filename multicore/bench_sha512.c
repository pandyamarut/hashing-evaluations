#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <openssl/sha.h>
#include <pthread.h>

#define MESSAGE_SIZE 1024*1024   // Size of input message in bytes
#define NUM_ITERATIONS 10000     // Number of iterations to run
#define NUM_THREADS 4            // Number of threads to use

struct thread_data {
    uint8_t *message;
    int iterations;
    double *hash_rate;
};

void *thread_func(void *arg) {
    struct thread_data *data = (struct thread_data *) arg;
    uint8_t digest[SHA512_DIGEST_LENGTH];
    struct timeval start_time, end_time;
    double total_time;

    // Measure SHA-512 hash rate
    gettimeofday(&start_time, NULL);
    for (int i = 0; i < data->iterations; i++) {
        SHA512(data->message, MESSAGE_SIZE, digest);
    }
    gettimeofday(&end_time, NULL);

    // Calculate total time and hash rate
    total_time = ((end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
                  (end_time.tv_usec - start_time.tv_usec)) / 1000000.0;
    *data->hash_rate = data->iterations / total_time;

    return NULL;
}

int main(int argc, char *argv[]) {
    uint8_t *message = malloc(MESSAGE_SIZE);
    struct timeval start_time, end_time;
    double total_time;
    double hash_rate = 0;

    // Generate random input message
    srand(time(NULL));
    for (int i = 0; i < MESSAGE_SIZE; i++) {
        message[i] = rand() % 256;
    }

    // Create threads and measure hash rate
    pthread_t threads[NUM_THREADS];
    struct thread_data thread_data[NUM_THREADS];
    int iterations_per_thread = NUM_ITERATIONS / NUM_THREADS;
    gettimeofday(&start_time, NULL);
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].message = message;
        thread_data[i].iterations = iterations_per_thread;
        thread_data[i].hash_rate = &hash_rate;
        pthread_create(&threads[i], NULL, thread_func, &thread_data[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    gettimeofday(&end_time, NULL);

    // Calculate total time
    total_time = ((end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
                  (end_time.tv_usec - start_time.tv_usec)) / 1000000.0;

    // Print results
    printf("SHA-512 hash rate: %.2f hashes/s\n", hash_rate * NUM_THREADS);
    printf("Total time: %.2f s\n", total_time);

    free(message);
    return 0;
}
