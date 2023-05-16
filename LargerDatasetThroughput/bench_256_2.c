#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <openssl/sha.h>
#include <pthread.h>

#define INPUT_FILE_PATH "path_to_10gb_file"  // Replace with the actual file path
#define READ_BUFFER_SIZE 32  // Number of bytes to read from the file
#define NUM_ITERATIONS 10000  // Number of iterations to run
#define NUM_THREADS 4  // Number of threads to use

struct thread_data {
    FILE *input_file;
    int iterations;
    double *hash_rate;
};

void *thread_func(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    uint8_t buffer[READ_BUFFER_SIZE];
    uint8_t digest[SHA256_DIGEST_LENGTH];
    struct timeval start_time, end_time;
    double total_time;

    // Measure SHA-256 hash rate
    gettimeofday(&start_time, NULL);
    for (int i = 0; i < data->iterations; i++) {
        fread(buffer, sizeof(uint8_t), READ_BUFFER_SIZE, data->input_file);
        SHA256(buffer, READ_BUFFER_SIZE, digest);
    }
    gettimeofday(&end_time, NULL);

    // Calculate total time and hash rate
    total_time = ((end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
                  (end_time.tv_usec - start_time.tv_usec)) / 1000000.0;
    *data->hash_rate = data->iterations / total_time;

    return NULL;
}

int main(int argc, char *argv[]) {
    FILE *input_file = fopen(INPUT_FILE_PATH, "rb");
    if (input_file == NULL) {
        printf("Failed to open input file.\n");
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
        thread_data[i].input_file = input_file;
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
    printf("SHA-256 hash rate: %.2f hashes/s\n", hash_rate * NUM_THREADS);
    printf("Total time: %.2f s\n", total_time);

    fclose(input_file);
    return 0;
}
