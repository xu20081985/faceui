#include <malloc.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "queue.h"

int q_init(queue_t** q) 
{
    if (*q)
        return -1;
    *q = malloc(sizeof(queue_t));
    if (!(*q))
        return -1;
    (*q)->head = malloc(sizeof(node_t));
    if (!((*q)->head))
    {
        free(*q);
        *q = 0;
        return -1;
    }
    (*q)->head->last = (*q)->head;
    (*q)->head->next = (*q)->head;
	memset(&(*q)->head->d, 0, sizeof(data_t));
    (*q)->size = 0;
    pthread_mutex_init(&(*q)->mutex, NULL);//队列里面有互斥量
    pthread_cond_init(&(*q)->condv, NULL);
	
	return 0;
}

int q_push_head(queue_t* q, data_t *d) 
{
    node_t* node;
    node_t* head;

	if (!d)
		return -1;
	
    if (!q)
        return -1;
    node = (node_t*) malloc(sizeof(node_t));
    if (!node)
        return -1;
    node->d = *d;
    pthread_mutex_lock(&q->mutex);
    q->size++;
    head = q->head;
    node->next = head->next;
    head->next->last = node;
    node->last = head;
    head->next = node;
    pthread_cond_signal(&q->condv);
    pthread_mutex_unlock(&q->mutex);

    return 0;
}

int q_push_tail(queue_t* q, data_t *d) 
{
    node_t* node;
    node_t* head;

	if (!d)
		return -1;
    if (!q)
        return -1;
    node = (node_t*) malloc(sizeof(node_t));
    if (!node)
        return -1;
    node->d = *d;
    pthread_mutex_lock(&q->mutex);
    q->size++;
    head = q->head;
    node->next = head;
    head->last->next = node;
    node->last = head->last;
    head->last = node;
    pthread_cond_signal(&q->condv);
    pthread_mutex_unlock(&q->mutex);

    return 0;
}

data_t q_pop_head(queue_t* q, const int sec, const int msec) 
{
    node_t* node;
    node_t* head;
    data_t result = {0};

	struct timespec timeout;
	struct timeval now;
	gettimeofday(&now, NULL);
	timeout.tv_sec = now.tv_sec + sec;
    timeout.tv_nsec = (now.tv_usec * 1000) + (msec * 1000 * 1000);
	
    if (!q)
    {
        return result;
    }
    pthread_mutex_lock(&q->mutex);
	
    if (q->size <= 0)
	{
		pthread_cond_timedwait(&q->condv, &q->mutex, &timeout);
		//result = 0;
	}
	if (q->size <= 0)
	{

	}
    else 
    {
        q->size--;
        head = q->head;
        node = head->next;
        head->next = node->next;
        node->next->last = head;
        result = node->d;
        free(node);
		node = NULL;
    }
    pthread_mutex_unlock(&q->mutex);

    return result;
}

data_t q_pop_tail(queue_t* q, const int sec, const int msec) 
{
    node_t* node;
    node_t* head;
    data_t result = {0};

	struct timespec timeout;
	struct timeval now;
	gettimeofday(&now, NULL);
	timeout.tv_sec = now.tv_sec + sec;
    timeout.tv_nsec = (now.tv_usec * 1000) + (msec * 1000 * 1000);
	
    if (!q)
        return result;
    pthread_mutex_lock(&q->mutex);
	
    if (q->size <= 0) 
    {
    	pthread_cond_timedwait(&q->condv, &q->mutex, &timeout);
        //result = 0;
    }
	if (q->size <= 0)
	{

	}
    else 
    {
        q->size--;
        head = q->head;
        node = head->last;
        head->last = node->last;
        node->last->next = head;
        result = node->d;
        free(node);
		node = NULL;
    }
    pthread_mutex_unlock(&q->mutex);

    return result;
}

int q_size(queue_t* q) 
{
    int size;

    if (!q)
        return -1;
    pthread_mutex_lock(&q->mutex);
    size = q->size;
    pthread_mutex_unlock(&q->mutex);

    return size;
}

void q_abort(queue_t* q) 
{
   node_t* node;
    void* ptr;

    if (!q)
        return;
    pthread_mutex_lock(&q->mutex);
    while (q->size > 0) 
    {
        node = q->head->next;
        ptr = node->d.ptr;
        if (ptr)
        {
            free(ptr);
			ptr = NULL;
        }	
        q->head->next = node->next;
        free(node);
		node = NULL;
        q->size--;
    }
    q->head->next = q->head;
    q->head->last = q->head;
    pthread_mutex_unlock(&q->mutex);
}

void q_wake(queue_t* q) 
{
    if (!q)
        return;
    pthread_mutex_lock(&q->mutex);
    if (!q->size)
        q->size = -1;
    pthread_cond_broadcast(&q->condv);
    pthread_mutex_unlock(&q->mutex);
}

void q_free(queue_t** q) 
{
    if (!(*q))
        return;
    q_abort(*q);
    pthread_mutex_destroy(&(*q)->mutex);
    pthread_cond_destroy(&(*q)->condv);
    free(*q);
    *q = 0;
}
