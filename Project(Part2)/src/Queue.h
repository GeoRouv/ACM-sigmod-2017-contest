#ifndef __QUEUE_H__
#define __QUEUE_H__

/*

Project team:
1) Stefou Theodoros 		1115201400193
2) Kokkinakos Panagiotis	1115201400069
3) Rouvalis Georgios		1115201400173

*/

typedef struct Queue_t* Queue;

typedef char Bool;

void 	QueueCreate(Queue* q);
void 	QueueDestroy(Queue* q);
void 	QueuePush(Queue q, void* data);
void 	QueuePushUnique(Queue q, void* data);
void* 	QueuePop(Queue q);
Bool 	QueueIsEmpty(Queue q);

#endif