#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Queue.h"

/*

Project team:
1) Stefou Theodoros 		1115201400193
2) Kokkinakos Panagiotis	1115201400069
3) Rouvalis Georgios		1115201400173

*/

typedef struct QueueNode
{
	void* data;
	struct QueueNode* next;

} QueueNode;

typedef struct Queue_t
{
	QueueNode* head;
	QueueNode* tail;

} Queue_t;

void QueueCreate(Queue* q)
{
	(*q) = malloc(sizeof(Queue_t));
	(*q)->head = NULL;
	(*q)->tail = NULL;
}

void QueueDestroy(Queue* q)
{
	QueueNode* tmp = (*q)->head;
	while(tmp){
		QueueNode* next = tmp->next;
		free(tmp);
		tmp = next;
	}

	free(*q);
	(*q) = NULL;
	return;
}

void QueuePush(Queue q, void* data)
{
	/* Push to empty list */
	if(q->head == NULL){
		q->head = malloc(sizeof(QueueNode));
		q->tail = q->head;
		q->head->data = data;
		q->head->next = NULL;
		return;
	}

	/* Not empty */
	q->tail->next = malloc(sizeof(QueueNode));
	q->tail = q->tail->next;
	q->tail->data = data;
	q->tail->next = NULL;
	return;
}

void QueuePushFront(Queue q, void* data)
{
	QueueNode* prev = q->head;
	QueueNode* new = malloc(sizeof(QueueNode));
	new ->data = data;
	new->next = prev;
	q->head = new;
}

void* QueuePop(Queue q)
{
	/* Destroy the first node and return its data */
	if(q->head == NULL) return NULL;

	void* ret = q->head->data;
	QueueNode* tmp = q->head;
	q->head = q->head->next;
	free(tmp);
	if(q->head == NULL) q->tail = NULL;

	return ret;
}

void QueuePushUnique(Queue q, void* data)
{
	if(q->head == NULL){
		q->head = malloc(sizeof(QueueNode));
		q->tail = q->head;
		q->head->data = data;
		q->head->next = NULL;
		return;
	}

	QueueNode* tmp = q->head;
	int found = 0;
	while(tmp->next){
		if( !strcmp((char*)tmp->data, (char*)data) ){
			found = 1;
			break;
		}
		tmp = tmp->next;
	}
	if( !strcmp((char*)tmp->data, (char*)data) )
		found = 1;


	/* If already in Queue, no need to do anything */
	if(found){
		free(data);
		return;
	}

	/* Else append to list */
	q->tail->next = malloc(sizeof(QueueNode));
	q->tail = q->tail->next;
	q->tail->data = data;
	q->tail->next = NULL;

}

Bool QueueIsEmpty(Queue q)
{
	return q->head == NULL;
}
