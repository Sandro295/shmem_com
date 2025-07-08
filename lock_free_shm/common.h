#ifndef COMMON_H
#define COMMON_H

#include <stdatomic.h>

#define SHM_NAME "/my_shm_lock_free"
#define MSG_SIZE 256
#define INT_ARRAY_SIZE 10
#define DOUBLE_ARRAY_SIZE 10

struct channel {
    _Atomic unsigned int sequence;
    char message[MSG_SIZE];
};

// The new struct for the random data payload
struct random_data_payload {
    int int_array[INT_ARRAY_SIZE];
    double double_array[DOUBLE_ARRAY_SIZE];
};

struct shared_data {
    struct channel channel1; // producer to consumer
    struct channel channel2; // consumer to producer

    // Sequence counter for lock-free access to the random data
    _Atomic unsigned int random_data_seq;

    // The actual random data payload
    struct random_data_payload random_data;
};

#endif // COMMON_H
