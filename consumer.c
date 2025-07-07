#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

#include "common.h"

int main() {
    int shm_fd;
    shm_data_t *shm_ptr;
    sem_t *sem_mutex, *sem_p_to_c, *sem_c_to_p;

    srand(time(NULL)); // Seed for random number generation

    // Open or create shared memory
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    // Configure size of shared memory (only if created by consumer)
    if (ftruncate(shm_fd, sizeof(shm_data_t)) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    // Map shared memory to process address space
    shm_ptr = mmap(0, sizeof(shm_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // Open semaphores
    sem_mutex = sem_open(SEM_MUTEX_NAME, O_CREAT, 0666, 1);
    if (sem_mutex == SEM_FAILED) {
        perror("sem_open mutex");
        exit(EXIT_FAILURE);
    }

    sem_p_to_c = sem_open(SEM_PRODUCER_TO_CONSUMER_NAME, O_CREAT, 0666, 0);
    if (sem_p_to_c == SEM_FAILED) {
        perror("sem_open p_to_c");
        exit(EXIT_FAILURE);
    }

    sem_c_to_p = sem_open(SEM_CONSUMER_TO_PRODUCER_NAME, O_CREAT, 0666, 0);
    if (sem_c_to_p == SEM_FAILED) {
        perror("sem_open c_to_p");
        exit(EXIT_FAILURE);
    }

    printf("Consumer: Ready for bidirectional communication. Waiting for messages...\n");

    while (1) { // Loop indefinitely
        // Consumer waits for producer's message
        sem_wait(sem_p_to_c); // Wait for producer to write
        sem_wait(sem_mutex); // Lock shared memory

        if (shm_ptr->sender_id == SENDER_PRODUCER) {
            switch (shm_ptr->type) {
                case MSG_TYPE_QUERY_RANDOM:
                    printf("Consumer: Received query: '%s'\n", shm_ptr->buffer);
                    sprintf(shm_ptr->buffer, "%d", rand()); // Just put the random number
                    break;
                case MSG_TYPE_DISPLAY_STRING:
                    printf("Consumer: Displaying string: '%s'\n", shm_ptr->buffer);
                    sprintf(shm_ptr->buffer, "Acknowledged: '%s'", shm_ptr->buffer);
                    break;
                default:
                    sprintf(shm_ptr->buffer, "Unknown message type");
                    break;
            }
            shm_ptr->sender_id = SENDER_CONSUMER;
            // Set the type of the response message
            if (shm_ptr->type == MSG_TYPE_QUERY_RANDOM) {
                // If it was a query, the response is also a query type (containing the random data)
                shm_ptr->type = MSG_TYPE_QUERY_RANDOM;
            } else if (shm_ptr->type == MSG_TYPE_DISPLAY_STRING) {
                // If it was a display string, the response is an acknowledgment
                shm_ptr->type = MSG_TYPE_DISPLAY_STRING;
            }
            if (shm_ptr->type == MSG_TYPE_QUERY_RANDOM) {
                printf("Consumer: Sent response to query.\n");
            } else {
                printf("Consumer: Sent response '%s'\n", shm_ptr->buffer);
            }
        }
        sem_post(sem_mutex); // Unlock shared memory
        sem_post(sem_c_to_p); // Signal producer
    }

    // Clean up (This part will not be reached in an infinite loop, but good practice)
    sem_close(sem_mutex);
    sem_close(sem_p_to_c);
    sem_close(sem_c_to_p);
    munmap(shm_ptr, sizeof(shm_data_t));
    close(shm_fd);

    printf("Consumer: Exiting.\n");

    return 0;
}