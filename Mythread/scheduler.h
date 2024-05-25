#pragma once
#include<pthread.h>
#include <sys/socket.h>
#include <queue>
#include "worker.h"
#include "payload.h"

class Scheduler {
    pthread_t w_thread;
    Worker** workers;
    int n_workers;
    std::queue<Payload *> Q;
    int curidx;

    public:
        pthread_mutex_t *lock;
        pthread_cond_t *cv;
        Scheduler(Worker**, int, pthread_mutex_t*, pthread_cond_t*);
        void push(Payload *);
        int init();
        void main_loop();
};

static void* schedule_wrapper(void *);