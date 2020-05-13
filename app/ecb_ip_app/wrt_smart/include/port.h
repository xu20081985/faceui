#ifndef PORT_H
#define PORT_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ortp/logging.h"
#include "ortp/port.h"
#include "ortp/str_utils.h"



#ifdef __cplusplus
extern "C"{
#endif



#define ms_thread_t		    ortp_thread_t
#define ms_thread_create 	ortp_thread_create
#define ms_thread_join		ortp_thread_join
#define ms_thread_exit		ortp_thread_exit


#define ms_malloc	ortp_malloc
#define ms_malloc0	ortp_malloc0
#define ms_realloc	ortp_realloc
#define ms_free		ortp_free
#define ms_strdup	ortp_strdup
#define ms_strndup  ortp_strndup
#define ms_new		ortp_new
#define ms_new0		ortp_new0

#define ms_strdup_printf ortp_strdup_printf

#define ms_message  ortp_message
#define ms_warning	ortp_warning
#define ms_error	ortp_error
#define ms_fatal	ortp_fatal





/*****************************MSLIST***********************/

struct _MSList {
	struct _MSList *next;
	struct _MSList *prev;
	void *data;
};

typedef struct _MSList MSList;

MSList *ms_list_append_link(MSList *elem, MSList *new_elem);
MSList * ms_list_append(MSList *elem, void * data);
MSList * ms_list_prepend(MSList *elem, void * data);
MSList * ms_list_free(MSList *elem);
MSList * ms_list_concat(MSList *first, MSList *second);
MSList * ms_list_remove(MSList *first, void *data);
int ms_list_size(const MSList *first);
void ms_list_for_each(const MSList *list, void (*func)(void *));
void ms_list_for_each2(const MSList *list, void (*func)(void *, void *), void *user_data);
MSList *ms_list_remove_link(MSList *list, MSList *elem);
MSList *ms_list_find(MSList *list, void *data);
MSList *ms_list_find_custom(MSList *list, int (*compare_func)(const void *, const void*), void *user_data);
void * ms_list_nth_data(const MSList *list, int index);
int ms_list_position(const MSList *list, MSList *elem);
int ms_list_index(const MSList *list, void *data);
MSList *ms_list_insert_sorted(MSList *list, void *data, int (*compare_func)(const void *, const void*));
MSList *ms_list_insert(MSList *list, MSList *before, void *data);
MSList *ms_list_copy(const MSList *list);
#define ms_list_next(elem) ((elem)->next)
#define ms_list_next(elem) ((elem)->next)


/*****************Log*******************/


#ifdef __cplusplus
}
#endif

#endif