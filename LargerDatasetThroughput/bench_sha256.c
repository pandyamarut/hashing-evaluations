#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <openssl/sha.h>
#include <pthread.h>

#define CHUNK_SIZE 32              // Size of each chunk in bytes
#define NUM_THREADS 4              // Number of threads to use
#define NUM_ITERATIONS 10000000    // Number of iterations to run

struct thread_data {
    FILE *file;
    int iterations;
    double *hash_rate;
};

void *thread_func(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    uint8_t chunk[CHUNK_SIZE];
    uint8_t digest[SHA256_DIGEST_LENGTH];
    struct timeval start_time, end_time;
    double total_time;

    // Measure SHA-256 hash rate
    gettimeofday(&start_time, NULL);
    for (int i = 0; i < data->iterations; i++) {
        fseek(data->file, 0, SEEK_SET);  // Reset file position to the beginning

        while (fread(chunk, sizeof(uint8_t), CHUNK_SIZE, data->file) == CHUNK_SIZE) {
            SHA256(chunk, CHUNK_SIZE, digest);
        }
    }
    gettimeofday(&end_time, NULL);

    // Calculate total time and hash rate
    total_time = ((end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
                  (end_time.tv_usec - start_time.tv_usec)) / 1000000.0;
    *data->hash_rate = (data->iterations * ftell(data->file)) / CHUNK_SIZE / total_time;

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Error opening file: %s\n", filename);
        return 1;
    }

    struct timeval start_time, end_time;
    double total_time;
    double hash_rate = 0;

    // Create threads and measure hash rate
    pthread_t threads[NUM_THREADS];
    struct thread_data thread_data[NUM_THREADS];
    int iterations_per_thread = NUM_ITERATIONS / NUM_THREADS;
    gettimeofday(&start_time, NULL);
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].file = file;
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
    printf("SHA-256 hash rate: %.2f chunks/s\n", hash_rate * NUM_THREADS);
    printf("Total time: %.2f s\n", total_time);

    fclose(file);
    return 0;
}
