#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdatomic.h>
#include <time.h>
#include "common.h"

int main() {
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd, sizeof(struct shared_data)) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    struct shared_data *data = mmap(NULL, sizeof(struct shared_data),
                                  PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    atomic_init(&data->channel1.sequence, 0);
    atomic_init(&data->channel2.sequence, 0);
    atomic_init(&data->random_data_seq, 0);

    srand(time(NULL));
    unsigned int msg_seq = 1;

    while (1) {
        // Generate and write random data using a sequence lock
        unsigned int seq = atomic_load_explicit(&data->random_data_seq, memory_order_relaxed);
        atomic_store_explicit(&data->random_data_seq, seq + 1, memory_order_release); // Indicate writing

        for (int i = 0; i < INT_ARRAY_SIZE; i++) {
            data->random_data.int_array[i] = rand();
        }
        for (int i = 0; i < DOUBLE_ARRAY_SIZE; i++) {
            data->random_data.double_array[i] = (double)rand() / RAND_MAX;
        }

        atomic_store_explicit(&data->random_data_seq, seq + 2, memory_order_release); // Indicate done writing

        // Check for messages from the producer
        unsigned int current_msg_seq = atomic_load_explicit(&data->channel1.sequence, memory_order_acquire);
        if (current_msg_seq == msg_seq) {
            printf("Received message: %s\n", data->channel1.message);
            if (strcmp(data->channel1.message, "exit") == 0) {
                break;
            }
            msg_seq++;
        }

        usleep(500000); // 500ms
    }

    munmap(data, sizeof(struct shared_data));
    shm_unlink(SHM_NAME);

    return 0;
}