#ifndef SHELL_H
#define SHELL_H

#include <pthread.h>

struct async {
    int (*proc)( void *data );
    void *data;
    int in;
    int out;
    pthread_t tid;
    int proc_in;
    int proc_out;
};

int start_async(struct async *async);
int finish_async(struct async *async);

#endif
