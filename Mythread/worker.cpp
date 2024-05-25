#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <string>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "worker.h"
#include "payload.h"
#include "rocksdb/db.h"
#define BUF_SIZE 128

Worker::Worker(int n_ID, int fd, pthread_mutex_t* inlock, pthread_cond_t* cond, rocksdb::DB* indb, std::mutex* inmut, std::queue<Payload *>* inQ, int n_w, Worker** in_w) {
    ID = n_ID;
    sockfd = fd;
    lock = inlock;
    cv = cond;
    db = indb;
    Q_lock = inmut;
    Main_Q = inQ;
    num_workers = n_w;
    workers = in_w;
    pl = NULL;
}

inline int Worker::parselen() {
    int in_i = 1;
    while(1) {
        if(pl -> buf[in_i] == ' ') {
            break;
        }
        in_i++;
    }
    pl -> buf[in_i] = 0;
    i = atoi(&(pl -> buf[1]));
    key = &(pl -> buf[in_i + 1]);
    if(pl -> buf[0] == 'P') {
        pl -> buf[in_i + i + 1] = 0;
        value = &(pl -> buf[in_i + i + 2]);
        return 1;
    }
    return 0;
}

int Worker::work() {
    while(1) {
        if(pl != NULL) {
        /*
        while(Q.size()) {  
            pl = Q.front();
            Q.pop();
            */
            if(parselen()) {
                rocksdb::Status status = db->Put(rocksdb::WriteOptions(), key, value);
                assert(status.ok());
                buf[0] = 'O';
                buf[1] = 'K';
                buf[2] = 0;
                sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&(pl -> addr), sizeof(pl -> addr));
            }
            else{
                std::string ret;
                rocksdb::Status status = db->Get(rocksdb::ReadOptions(), key, &ret);
                if(status.IsNotFound()) {
                    strcpy(buf, "NO SUCH KEY");
                    buf[11] = 0;
                }
                else if(status.ok()) {
                    strcpy(buf, ret.c_str());
                }
                else {
                    strcpy(buf, "ERROR IN DB");
                    buf[11] = 0;
                }
                sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&(pl -> addr), sizeof(pl -> addr));
            }
            //printf("DELETE PL %d : %p\n", ID, pl);
            delete(pl);
            pl = NULL;
        }
        if(Q_lock->try_lock()) {
            /*
            if(Q.empty()) {
                bool end = Main_Q -> empty();
                while(!end) {
                    int minval = workers[0] -> size();
                    for(int i = 1 ; i < num_workers ; i++) 
                        minval = std::min(minval, workers[i] -> size());
                    for(int i = 0 ; i < num_workers ; i++) {
                        if(workers[i] -> size() <= minval) {
                            if(i == ID) Q.push(Main_Q -> front());
                            else workers[i] -> push(Main_Q -> front());
                            Main_Q -> pop();
                            if(Main_Q -> empty()) {
                                end = 1;
                                break;
                            }
                        }
                    }
                }
            }
            */
            if(pl == NULL) {
                for(int i = 0 ; i < num_workers ; i++) {
                    if(workers[i] -> pl == NULL) {
                        if(Main_Q -> empty()) break;
                        workers[i] -> pl = Main_Q -> front();
                        Main_Q -> pop();
                    }
                }
            }
            Q_lock -> unlock();
        }
        /*
        else {
            pthread_mutex_lock(lock);
            pthread_cond_wait(cv, lock);
            pthread_mutex_unlock(lock);
        }
        */
    }
    return 0;
}

void Worker::push(Payload* in_pl) {
    Q.push(in_pl);
}

int Worker::init() {
    buf = (char *)malloc(BUF_SIZE * sizeof(char));
    if(pthread_create(&w_thread, NULL, &work_wrapper, this) < 0) {
        printf("Thread not created\n");
    }
    return 0;
}

int Worker::size() {
    return Q.size();
}

static void* work_wrapper(void *arg) {
    reinterpret_cast<Worker *>(arg) -> work();
    return 0;
}