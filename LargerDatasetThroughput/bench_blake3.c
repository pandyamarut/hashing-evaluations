#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include "blake3.h"
#include <pthread.h>

#define CHUNK_SIZE 32              // Size of each chunk in bytes
#define NUM_THREADS 4              // Number of threads to use
#define NUM_ITERATIONS 10000000    // Number of iterations to run

struct thread_args {
    FILE *file;
};

void *hash_thread(void *arg) {
    struct thread_args *args = (struct thread_args *) arg;
    uint8_t chunk[CHUNK_SIZE];
    uint8_t digest[BLAKE3_OUT_LEN];
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        fseek(args->file, 0, SEEK_SET);  // Reset file position to the beginning
        
        while (fread(chunk, sizeof(uint8_t), CHUNK_SIZE, args->file) == CHUNK_SIZE) {
            blake3_hasher_update(&hasher, chunk, CHUNK_SIZE);
        }
    }
    
    blake3_hasher_finalize(&hasher, digest, BLAKE3_OUT_LEN);
    pthread_exit(NULL);
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

    pthread_t threads[NUM_THREADS];
    struct thread_args args[NUM_THREADS];
    struct timeval start_time, end_time;
    double total_time;

    // Measure BLAKE3 hash rate
    gettimeofday(&start_time, NULL);
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].file = file;
        pthread_create(&threads[i], NULL, hash_thread, (void *)&args[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    gettimeofday(&end_time, NULL);

    // Calculate total time and hash rate
    total_time = ((end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
                  (end_time.tv_usec - start_time.tv_usec)) / 1000000.0;
    double hash_rate = (NUM_ITERATIONS * ftell(file)) / CHUNK_SIZE / total_time;

    // Print results
    printf("BLAKE3 hash rate: %.2f chunks/s\n", hash_rate);
    printf("Total time: %.2f s\n", total_time);

    fclose(file);
    return 0;
}
