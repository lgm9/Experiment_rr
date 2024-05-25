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
    int num_workers;
    Payload* pl;
    pthread_t w_thread;
    std::queue<Payload *> *Main_Q;
    std::queue<Payload *> Q;
    rocksdb::DB* db;
    std::mutex* Q_lock;
    Worker** workers;
    int i;
    inline int parselen();

    public:
        pthread_mutex_t *lock;
        pthread_cond_t *cv;
        Worker(int, int, pthread_mutex_t*, pthread_cond_t*, rocksdb::DB*, std::mutex*, std::queue<Payload *>*, int, Worker**);
        int work();
        int init();
        int size();
        void push(Payload *);
        ~Worker() {
            free(buf);
        }
};

static void* work_wrapper(void *);