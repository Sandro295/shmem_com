#ifndef COMMON_H
#define COMMON_H

#include <semaphore.h>

#define SHM_NAME "/my_shared_memory"
#define SEM_MUTEX_NAME "/sem_mutex"
#define SEM_PRODUCER_TO_CONSUMER_NAME "/sem_p_to_c"
#define SEM_CONSUMER_TO_PRODUCER_NAME "/sem_c_to_p"
#define BUFFER_SIZE 256

// Sender IDs
#define SENDER_PRODUCER 0
#define SENDER_CONSUMER 1

// Message Types
enum message_type {
    MSG_TYPE_QUERY_RANDOM,
    MSG_TYPE_DISPLAY_STRING
};

typedef struct {
    char buffer[BUFFER_SIZE];
    int sender_id; // 0 for producer, 1 for consumer
    enum message_type type; // Type of message
} shm_data_t;

#endif // COMMON_H
