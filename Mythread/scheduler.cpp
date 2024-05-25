#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "scheduler.h"
#include "payload.h"
#define BUF_SIZE 128

Scheduler::Scheduler(Worker** ws, int n_w, pthread_mutex_t* inlock, pthread_cond_t* cond) {
    workers = ws;
    n_workers = n_w;
    lock = inlock;
    cv = cond;
}

void Scheduler::main_loop() {
    while(1) {
        if(Q.empty()) {
            pthread_mutex_lock(lock);
            pthread_cond_wait(cv, lock);
            pthread_mutex_unlock(lock);
        }
        while(!Q.empty()) {
            if(workers[curidx] -> pl == NULL) {
                workers[curidx] -> pl = Q.front();
                Q.pop();
                pthread_cond_signal(workers[curidx] -> cv);
            }
            curidx = (curidx + 1) % n_workers;
        }
    }
}

void Scheduler::push(Payload* pl) {
    Q.push(pl);
}

int Scheduler::init() {
    curidx = 0;
    if(pthread_create(&w_thread, NULL, &schedule_wrapper, this) < 0) {
        printf("Thread not created\n");
    }
    return 0;
}

static void* schedule_wrapper(void *arg) {
    reinterpret_cast<Scheduler *>(arg) -> main_loop();
    return 0;
}