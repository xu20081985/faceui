#ifndef QUEUE_H
#define QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

typedef struct tagData
{
	int flag;
	void *ptr;
}data_t;
typedef struct node {
    struct node* last;
    struct node* next;
    data_t d;
}node_t;

typedef struct queue {
    pthread_mutex_t mutex;
    pthread_cond_t condv;
    node_t* head;
    int size;
}queue_t;

int q_init(queue_t** q);
int q_push_head(queue_t* q, data_t *d);
int q_push_tail(queue_t* q, data_t *d);
data_t q_pop_head(queue_t* q, const int sec, const int msec );
data_t q_pop_tail(queue_t* q, const int sec, const int msec );
int q_size(queue_t* q);
void q_abort(queue_t* q);
void q_wake(queue_t* q);
void q_free(queue_t** q);

extern int gettimeofday(struct timeval *tv, struct timezone *tz);

#ifdef __cplusplus
}
#endif


#endif
