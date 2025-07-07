#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>

#include "common.h"

int main(int argc, char *argv[]) {
    int shm_fd;
    shm_data_t *shm_ptr;
    sem_t *sem_mutex, *sem_p_to_c, *sem_c_to_p;

    enum message_type msg_type;
    char *string_to_display = NULL;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <query|string> [message_content]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "query") == 0) {
        msg_type = MSG_TYPE_QUERY_RANDOM;
    } else if (strcmp(argv[1], "string") == 0) {
        msg_type = MSG_TYPE_DISPLAY_STRING;
        if (argc < 3) {
            fprintf(stderr, "Usage: %s string <message_content>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        string_to_display = argv[2];
    } else {
        fprintf(stderr, "Invalid message type. Use 'query' or 'string'.\n");
        exit(EXIT_FAILURE);
    }

    // Open shared memory
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    // Map shared memory to process address space
    shm_ptr = mmap(0, sizeof(shm_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // Open semaphores
    sem_mutex = sem_open(SEM_MUTEX_NAME, O_CREAT, 0666, 1); // Mutex starts unlocked
    if (sem_mutex == SEM_FAILED) {
        perror("sem_open mutex");
        exit(EXIT_FAILURE);
    }

    sem_p_to_c = sem_open(SEM_PRODUCER_TO_CONSUMER_NAME, O_CREAT, 0666, 0); // Producer to consumer starts locked
    if (sem_p_to_c == SEM_FAILED) {
        perror("sem_open p_to_c");
        exit(EXIT_FAILURE);
    }

    sem_c_to_p = sem_open(SEM_CONSUMER_TO_PRODUCER_NAME, O_CREAT, 0666, 0); // Consumer to producer starts locked
    if (sem_c_to_p == SEM_FAILED) {
        perror("sem_open c_to_p");
        exit(EXIT_FAILURE);
    }

    printf("Producer: Ready for bidirectional communication.\n");

    for (int i = 0; i < 5; ++i) {
        // Producer writes
        sem_wait(sem_mutex); // Lock shared memory
        shm_ptr->sender_id = SENDER_PRODUCER;
        shm_ptr->type = msg_type;

        if (msg_type == MSG_TYPE_QUERY_RANDOM) {
            sprintf(shm_ptr->buffer, "Query random data request %d", i + 1);
            printf("Producer: Sent query request '%s'\n", shm_ptr->buffer);
        } else if (msg_type == MSG_TYPE_DISPLAY_STRING) {
            strncpy(shm_ptr->buffer, string_to_display, BUFFER_SIZE - 1);
            shm_ptr->buffer[BUFFER_SIZE - 1] = '\0';
            printf("Producer: Sent string '%s'\n", shm_ptr->buffer);
        }
        sem_post(sem_mutex); // Unlock shared memory
        sem_post(sem_p_to_c); // Signal consumer

        // Producer waits for consumer's response
        sem_wait(sem_c_to_p); // Wait for consumer to write
        sem_wait(sem_mutex); // Lock shared memory

        printf("Producer (DEBUG): Received response. sender_id: %d, type: %d, buffer: '%s'\n",
               shm_ptr->sender_id, shm_ptr->type, shm_ptr->buffer);

        if (shm_ptr->sender_id == SENDER_CONSUMER) {
            if (shm_ptr->type == MSG_TYPE_QUERY_RANDOM) {
                printf("Producer: Received random data: '%s'\n", shm_ptr->buffer);
            } else if (shm_ptr->type == MSG_TYPE_DISPLAY_STRING) {
                printf("Producer: Received acknowledgment: '%s'\n", shm_ptr->buffer);
            } else {
                printf("Producer: Received unknown response: '%s'\n", shm_ptr->buffer);
            }
        }
        sem_post(sem_mutex); // Unlock shared memory

        sleep(1); // Simulate work
    }

    // Clean up
    sem_close(sem_mutex);
    sem_close(sem_p_to_c);
    sem_close(sem_c_to_p);
    munmap(shm_ptr, sizeof(shm_data_t));
    close(shm_fd);

    printf("Producer: Exiting.\n");

    return 0;
}
