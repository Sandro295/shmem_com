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

    srand(time(NULL));
    unsigned int expected_seq = 1;

    while (1) {
        atomic_store_explicit(&data->random_value, rand(), memory_order_release);

        unsigned int current_seq = atomic_load_explicit(&data->channel1.sequence, memory_order_acquire);
        if (current_seq == expected_seq) {
            printf("Received message: %s\n", data->channel1.message);

            if (strcmp(data->channel1.message, "exit") == 0) {
                break;
            }

            expected_seq++;
        }
        usleep(100000); // 100ms
    }

    munmap(data, sizeof(struct shared_data));
    shm_unlink(SHM_NAME);

    return 0;
}