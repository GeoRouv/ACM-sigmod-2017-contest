#ifndef __JOBSCHEDULER_H__
#define __JOBSCHEDULER_H__

#include <pthread.h>
#include "AssistingFunctions.h"
#include "Queue.h"

#define NUMTHREADS 4

typedef char Bool;

typedef struct mut{
	pthread_mutex_t mutex;
	pthread_cond_t   cond;
	int value;

}mut;

typedef struct Job{
	void* (*FuncPtr)(void* arg);
	void* args;
	int id; /* Every time we execute, we also set the id counter to 0 */

}Job;

typedef struct JobQueue{
	Queue q;
	pthread_mutex_t access_mutex;
	int jobs;

}JobQueue;

typedef struct JobScheduler{
	int num_threads;
	JobQueue q; /* a queue that holds submitted jobs / tasks */
	pthread_t* tids; /* execution threads */
	mut* start;
	int jobs_done; /*How many jobs have been completed in the batch*/
	pthread_mutex_t wait_mut;
	pthread_cond_t wait_cond;

}JobScheduler;

Job* Job_Create(void* (*FuncPtr)(void* arg),void* args,int id);
JobScheduler* Init_Scheduler();
void Submit_Job(JobScheduler* sch,void* (*func_ptr)(void*),void* args,int id);
void Execute_All_Jobs(JobScheduler* sch,Queue* burst);
void Wait_All_Jobs_Finish(JobScheduler* sch); /* Waits all submitted jobs to finish */
Bool Destroy_Scheduler(JobScheduler* sch);


#endif 
