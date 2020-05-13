#ifndef __THREADPOOL_H
#define __THREADPOOL_H

#ifdef __cplusplus
extern"C"
{
#endif
  
/* 
*�̳߳����������к͵ȴ���������һ��CThread_worker 
*�������������������������һ������ṹ 
*/  
typedef struct worker  
{  
    /*�ص���������������ʱ����ô˺�����ע��Ҳ��������������ʽ*/  
    void *(*process) (void *arg);  
    void *arg;/*�ص������Ĳ���*/  
    struct worker *next;  
  
} CThread_worker;  
  
  
  
/*�̳߳ؽṹ*/  
typedef struct  
{  
    pthread_mutex_t queue_lock;  
    pthread_cond_t queue_ready;  
  
    /*����ṹ���̳߳������еȴ�����*/  
    CThread_worker *queue_head;  
  
    /*�Ƿ������̳߳�*/  
    int shutdown;  
    pthread_t *threadid;  
    /*�̳߳�������Ļ�߳���Ŀ*/  
    int max_thread_num;  
    /*��ǰ�ȴ����е�������Ŀ*/  
    int cur_queue_size;
} CThread_pool;  

void  pool_init(int max_thread_num);
int  pool_add_worker(void *(*process)(void *arg), void *arg);
int  pool_destroy();
void *  thread_routine(void *arg);

#ifdef __cplusplus
}
#endif

#endif
