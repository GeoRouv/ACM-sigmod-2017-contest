#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "DynamicTrie.h"
#include "StaticTrie.h"
#include "Queue.h"
#include "AssistingFunctions.h"

#define INIT_NUM_CHILDREN 10 /*Initial arraySize of every node*/

/*

Project team:
1) Stefou Theodoros 		1115201400193
2) Kokkinakos Panagiotis	1115201400069
3) Rouvalis Georgios		1115201400173

*/

int main(int argc, char* argv[])
{

	if(argc != 5){
		print_usage(argv[0]);
		return -1;
	}

	int i,n;
	int flags_found = 0; /* More than two flags results in exit */
	Bool i_found = 0, q_found = 0;
	char* init_file;
	char* query_file;
	FILE* init;
	FILE* query;
	char* buffer;
	char* s;
	size_t buffer_size = 1024;
	int trie_kind; /*0 if the trie is dynamic, 1 if it is static*/

	for(i = 1; i < argc; i++){
		if( isFlag(argv[i]) ){
			/* Flag */
			if(i+1 < argc){
				if(isFlag(argv[i+1])){
					/* Two flags in a row */
					print_usage(argv[0]);
					return -1;
				}
			}
			else{
				/* Flag at the end */
				print_usage(argv[0]);
				return -1;
			}
			flags_found++;
			if(argv[i][1] == 'i'){
				if(i_found){
					print_usage(argv[0]);
					return -1;
				}
				i_found = 1;
				if(i+1 <  argc)
					init_file = argv[i+1];
				else{
					print_usage(argv[0]);
					return -1;
				}
			}
			else if(argv[i][1] == 'q'){
				if(q_found){
					print_usage(argv[0]);
					return -1;
				}
				q_found = 1;
				if(i+1 < argc)
					query_file = argv[i+1];
				else{
					print_usage(argv[0]);
					return -1;
				}
			}
			else{
				printf("%s: Unknown flag [%s]\n", argv[0], argv[i]);
				print_usage(argv[0]);
				return -1;
			}

		} 
	}

	if(flags_found != 2){
		printf("%s: Flags must be exactly 2!\n", argv[0]);
		print_usage(argv[0]);
		return -1;
	}

	#ifdef DEBUG
	printf("%s: args received-> init_file [%s], query_file: [%s]\n",
		argv[0], init_file, query_file);
	#endif

	init = fopen(init_file, "r");
	query = fopen(query_file, "r");
	if(!init){
		printf("%s: Could not open <init_file> [\"%s\"]\n", argv[0], init_file);
		return -1;
	}
	if(!query){
		fclose(init);
		printf("%s: Could not open <query_file> [\"%s\"]\n", argv[0], query_file);
		return -1;
	}

	/* This is the pointer to the tree. */
	void* trie;

	buffer = malloc(buffer_size*sizeof(char));

	/*Dynamic - Static check here. Use pointer to function to distinguish between different calls of the same function*/
	
	/* Declare the function pointers. */
	Bool (*TrieAddPtr)(void*,char*);
	Queue (*TrieQuestionPtr)(void*,char*);
	Bool (*TrieDeletePtr)(void*, char*);
	Bool (*TrieDestroyPtr)(void**);
	
	/* The first line must be either STATIC or DYNAMIC/ */
	n = getline(&buffer,&buffer_size,init);
	if(buffer[n-1] == '\n')
			buffer[n-1] = '\0';
	
	/* Assign the correct functions to the function pointers created above. */
	if(!strcmp(buffer, "DYNAMIC")){
		trie_kind = 0;
		TrieAddPtr = &DynamicTrieAdd;
		TrieQuestionPtr = &DynamicTrieQuestion;
		TrieDeletePtr = &DynamicTrieDelete;
		TrieDestroyPtr = &DynamicTrieDestroy;
		DynamicTrieCreate(&trie, INIT_NUM_CHILDREN);
	}
	else if(!strcmp(buffer, "STATIC")){
		trie_kind = 1;
		TrieAddPtr = &StaticTrieAdd;
		TrieQuestionPtr = &StaticTrieQuestion;
		TrieDeletePtr = &StaticTrieDelete;
		TrieDestroyPtr = &StaticTrieDestroy;
		StaticTrieCreate(&trie, INIT_NUM_CHILDREN);
	}
	else{
		printf("%s: Fatal error: First line must be either STATIC or DYNAMIC.\n", argv[0]);
		free(buffer);
		fclose(init);
		fclose(query);
		return -1;
	}
	
	/*Add every N-gram from the input file into the trie*/
	while((n = getline(&buffer,&buffer_size,init)) > 0){
		if(buffer[n-1] == '\n')
			buffer[n-1] = '\0';
		if(TrieAddPtr(trie,buffer)) printf("Failed to add N-gram into trie!\n");
	}

	/*If the trie is static, no add action can be performed after the compression*/ 
	if(trie_kind){
		StaticTrieCompress((StaticTrie)trie);
		TrieAddPtr = &StaticTrieAddForbid;
	}

	/*Queue for the queries in the same burst*/
	Queue burst;
	QueueCreate(&burst);
	Queue res_queue;

	while((n = getline(&buffer,&buffer_size,query)) > 0){
		if(buffer[n-1] == '\n')
			buffer[n-1] = '\0';

		n--;

		if(buffer[0] == 'A' && isspace(buffer[1])){
			/*We have to add an N_gram*/
			if(TrieAddPtr(trie,&buffer[2])){
				#ifdef DEBUG
				printf("Error in add!\n");
				#endif
			}
		}
		else if(buffer[0] == 'D' && isspace(buffer[1])){
			/*We have to delete an N_gram*/
			if(TrieDeletePtr(trie,&buffer[2])){
				#ifdef DEBUG
				printf("Error in delete!\n");
				#endif
			}
		}
		else if(buffer[0] == 'Q' && isspace(buffer[1])){
			/* We have to make a question */
			QueuePush(burst,(void*)TrieQuestionPtr(trie,&buffer[2]));
		}
		else if(buffer[0] == 'F'){

			/*Frequency array of n-grams in each burst*/
			ngram_freq* freq_array = malloc(10*sizeof(ngram_freq));
			int arr_size = 10;
			int arr_entries = 0;

			int k = 0;
			sscanf(buffer+1,"%d",&k);

			/*Pop every result-queue from the burst queue*/
			while((res_queue = (Queue)QueuePop(burst))){
				char* s = (char*) QueuePop(res_queue);
				printf("%s", s);
				if(!strcmp(s,"-1")){
					printf("\n");
					free(s);
					QueueDestroy(&res_queue);
					continue;
				}
				
				MapInsert(s,&freq_array,&arr_size,&arr_entries);

				/*Pop ngrams from the result-queue*/
				while( (s=(char*)QueuePop(res_queue)) ){
					printf("|%s", s);
					MapInsert(s,&freq_array,&arr_size,&arr_entries);
				}
				printf("\n");

				QueueDestroy(&res_queue);
			}

			if(arr_entries == 0){
				free(freq_array);
				continue;
			}

			if(k == 0){
				for(i = 0;i < arr_entries;i++)
					free(freq_array[i].ngram);
				free(freq_array);				
				continue;
			}

			/* Top k */

			/*Find max appearances that occur in the burst*/
			int max = -1;
			for(i = 0;i < arr_entries;i++)
				if(freq_array[i].times > max) max = freq_array[i].times;
			
			/*Each index of this array has a queue with every n-gram that has occured index-times*/
			Queue* reverse_array = calloc(max+1,sizeof(Queue));

			for(i = 0;i < arr_entries;i++){
				if(reverse_array[freq_array[i].times] == NULL) QueueCreate(&(reverse_array[freq_array[i].times]));
				QueuePush(reverse_array[freq_array[i].times],(void*)freq_array[i].ngram);
			}

			printf("Top: ");
			int popped = 0;
			int next_pop = max;

			char* str = (char*)QueuePop(reverse_array[next_pop]);
			printf("%s",str);

			popped++;

			if(QueueIsEmpty(reverse_array[next_pop])) next_pop--;
			while(reverse_array[next_pop] == NULL) next_pop--;

			while(popped < k && next_pop >= 1){
				str = (char*)QueuePop(reverse_array[next_pop]);
				printf("|%s",str);
				if(QueueIsEmpty(reverse_array[next_pop])) next_pop--;
				while(reverse_array[next_pop] == NULL) next_pop--;
				popped++;
			}

			printf("\n");

			/*Free arrays*/
			for(i = 1;i < max+1;i++)
				if(reverse_array[i]) QueueDestroy(&(reverse_array[i]));

			free(reverse_array);

			for(i = 0;i < arr_entries;i++)
				free(freq_array[i].ngram);

			free(freq_array);

		}
		else printf("Wrong Command!\n");
	}

	/*Destroy any remnants in the burst queue if no F was given at the end of the file*/
	while(res_queue = QueuePop(burst)){
		while((s = QueuePop(res_queue))) free(s);
		QueueDestroy(&res_queue);
	}
	QueueDestroy(&burst);
	TrieDestroyPtr(&trie);

	free(buffer);
	fclose(init);
	fclose(query);

	return 0;
}
