#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdatomic.h>
#include "common.h"

void query_random_data(struct shared_data *data) {
    struct random_data_payload local_data;
    unsigned int seq1, seq2;

    do {
        seq1 = atomic_load_explicit(&data->random_data_seq, memory_order_acquire);
        memcpy(&local_data, &data->random_data, sizeof(struct random_data_payload));
        seq2 = atomic_load_explicit(&data->random_data_seq, memory_order_acquire);
    } while (seq1 != seq2 || seq1 % 2 != 0);

    printf("Random Integers:\n");
    for (int i = 0; i < INT_ARRAY_SIZE; i++) {
        printf("  %d\n", local_data.int_array[i]);
    }

    printf("Random Doubles:\n");
    for (int i = 0; i < DOUBLE_ARRAY_SIZE; i++) {
        printf("  %f\n", local_data.double_array[i]);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <query|send> [message]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    struct shared_data *data = mmap(NULL, sizeof(struct shared_data),
                                  PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "query") == 0) {
        query_random_data(data);
    } else if (strcmp(argv[1], "send") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s send <message>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        unsigned int seq = atomic_load_explicit(&data->channel1.sequence, memory_order_relaxed) + 1;
        strcpy(data->channel1.message, argv[2]);
        atomic_store_explicit(&data->channel1.sequence, seq, memory_order_release);
    } else {
        fprintf(stderr, "Invalid command: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    munmap(data, sizeof(struct shared_data));
    return 0;
}
