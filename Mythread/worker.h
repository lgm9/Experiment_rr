#pragma once
#include<pthread.h>
#include <sys/socket.h>
#include <queue>
#include "payload.h"
#include "rocksdb/db.h"

class Worker {
    int ID;
    int sockfd;
    char* buf, *key, *value;
    pthread_t w_thread;
    rocksdb::DB* db;
    int i;
    inline int parselen();

    public:
        pthread_mutex_t *lock;
        Payload* pl;
        pthread_cond_t *cv;
        Worker(int, int, pthread_mutex_t*, pthread_cond_t*, rocksdb::DB*);
        int work();
        int init();
        ~Worker() {
            free(buf);
        }
};

static void* work_wrapper(void *);