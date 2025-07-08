#ifndef COMMON_H
#define COMMON_H

#include <stdatomic.h>

#define SHM_NAME "/my_shm_lock_free"
#define MSG_SIZE 256

struct channel {
    _Atomic unsigned int sequence;
    char message[MSG_SIZE];
};

struct shared_data {
    struct channel channel1; // producer to consumer
    struct channel channel2; // consumer to producer
    _Atomic int random_value; // random data from consumer
};

#endif // COMMON_H