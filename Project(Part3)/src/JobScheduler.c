#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "AssistingFunctions.h"
#include "Queue.h"
#include "JobScheduler.h"

extern int batch_questions;

int run; /*Threads still running*/
Queue* res;



Job* Job_Create(void* (*FuncPtr)(void* arg),void* args,int id){
	
	Job* j = malloc(sizeof(Job));
	j->FuncPtr = FuncPtr;
	j->args = args;
  	j->id = id;
	
	return j;
}

void* thread_routine(void* sch){

	JobScheduler* js = (JobScheduler*) sch;

	void* (*func_ptr)(void*);
	void*  args;
	Job* job;

	BloomFilter bf;
	BloomFilterCreate(&bf);

	while(run){
		
		pthread_mutex_lock(&(js->start->mutex));
		while(js->start->value == 0)
			pthread_cond_wait(&(js->start->cond),&(js->start->mutex));
		js->start->value = 0;
		pthread_mutex_unlock(&(js->start->mutex));

		if(run){

			/*Get the 1st job from the queue*/
			pthread_mutex_lock(&(js->q.access_mutex));

			job = (Job*) QueuePop(js->q.q);

			if(js->q.jobs == 1){
				js->q.jobs --;
			}
			else if (js->q.jobs > 1){ /*More than 1 jobs*/
				js->q.jobs --;
				
				pthread_mutex_lock(&(js->start->mutex));
				js->start->value  = 1;
				pthread_cond_signal(&(js->start->cond));
				pthread_mutex_unlock(&(js->start->mutex));
			}

			pthread_mutex_unlock(&(js->q.access_mutex));

			if (job != NULL){
				func_ptr = job->FuncPtr;
				((q_args*)job->args)->bf = bf;
				args = job->args;

				res[((q_args*)args)->q_id] = func_ptr(args);
			
				free(((q_args*)args)->string);
				free(args);
				free(job);

				pthread_mutex_lock(&(js->wait_mut));
				js->jobs_done++;
				pthread_cond_signal(&(js->wait_cond));
				pthread_mutex_unlock(&(js->wait_mut));
			}
		}
		else {
			/* Wake up the other threads and exit */
			pthread_mutex_lock(&(js->start->mutex));
			js->start->value  = 1;
			pthread_cond_signal(&(js->start->cond));
			pthread_mutex_unlock(&(js->start->mutex));
		}
	}

	BloomFilterDestroy(&bf);

	pthread_exit(NULL);
}

JobScheduler* Init_Scheduler(){

	int i;
	JobScheduler* js = malloc(sizeof(JobScheduler));
	js->num_threads = NUMTHREADS;
	//js->active_threads = 0;
	run = 1;

	js->jobs_done = 0;

	/*JobQueue Initialization*/
	QueueCreate(&(js->q.q));
	js->q.jobs = 0;
	pthread_mutex_init(&(js->q.access_mutex), NULL);
	/**************************/

	pthread_cond_init(&(js->wait_cond), NULL);
	pthread_mutex_init(&(js->wait_mut), NULL);

	js->start = malloc(sizeof(mut));
	pthread_cond_init(&(js->start->cond), NULL);
	pthread_mutex_init(&(js->start->mutex), NULL);
	js->start->value = 0;

	js->tids = malloc((js->num_threads)*sizeof(pthread_t));

	/* Thread init */
	for (i = 0; i < js->num_threads; i++){
		pthread_create(&(js->tids[i]), NULL,(void*)thread_routine, (void*)js);
		/*pthread_detach(js->tids[i]);*/
	}

	return js;
}

void Submit_Job(JobScheduler* sch,void* (*func_ptr)(void*),void* args,int id){

	Job* job = Job_Create(func_ptr,args,id);

	/*Add the job in the queue*/
	pthread_mutex_lock(&(sch->q.access_mutex));

	QueuePush(sch->q.q,(void*)job);

	sch->q.jobs++;

	pthread_mutex_unlock(&(sch->q.access_mutex));

	return;
}

void Execute_All_Jobs(JobScheduler* sch,Queue* burst){

	res = burst;

	/* Wake up the thread to proceed with the completion of jobs */
	pthread_mutex_lock(&(sch->start->mutex));
	sch->start->value = 1;
	pthread_cond_signal (&(sch->start->cond));
	pthread_mutex_unlock (&(sch->start->mutex));

	return;
}

/* Waits all submitted jobs to finish */
void Wait_All_Jobs_Finish(JobScheduler* sch){

	pthread_mutex_lock(&(sch->wait_mut));
	while(sch->jobs_done < batch_questions)
		pthread_cond_wait(&(sch->wait_cond),&(sch->wait_mut));
	pthread_mutex_unlock(&(sch->wait_mut));

	sch->jobs_done = 0;

	return;
} 

Bool Destroy_Scheduler(JobScheduler* js){

	int i;
	run = 0;

	/* All threads wake up. Time to exit */
	pthread_mutex_lock(&(js->start->mutex));
	js->start->value = 1;
	pthread_cond_signal (&(js->start->cond));
	pthread_mutex_unlock (&(js->start->mutex));

	for(i = 0; i < js->num_threads; i++) {
		void* r;
		pthread_join(js->tids[i], &r);
	}

	QueueDestroy(&(js->q.q));

	/* Cleanup */
	pthread_mutex_destroy(&(js->q.access_mutex));

	pthread_cond_destroy(&(js->start->cond));
	pthread_mutex_destroy(&(js->start->mutex));
	free(js->start);

	pthread_cond_destroy(&(js->wait_cond));
	pthread_mutex_destroy(&(js->wait_mut));

	free(js->tids);
	free(js);

	return 0;
}