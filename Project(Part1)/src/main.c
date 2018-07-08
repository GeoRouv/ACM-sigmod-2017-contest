#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "Trie.h"
#include "Queue.h"

#define INIT_NUM_CHILDREN 10 /*Initial arraySize of every node*/

/*

Project team:
1) Stefou Theodoros 		1115201400193
2) Kokkinakos Panagiotis	1115201400069
3) Rouvalis Georgios		1115201400173

*/

Bool isFlag(char* word)
{
	if(word[0] == '-' && strlen(word) == 2)
		return 1;
	return 0;
}

void print_usage(char* prog_name)
{
	printf("%s usage: ./ngrams -i <init_file> -q <query_file>\n", prog_name);
}

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
		printf("%s: Could not open <query_file> [\"%s\"]\n", argv[0], query_file);
		return -1;
	}

	/*Create a trie*/
	Trie trie;
	TrieCreate(&trie,INIT_NUM_CHILDREN);

	buffer = malloc(buffer_size*sizeof(char));

	/*Add every N-gram from the input file into the trie*/
	while((n = getline(&buffer,&buffer_size,init)) > 0){
		if(buffer[n-1] == '\n')
			buffer[n-1] = '\0';
		if(TrieAdd(trie,buffer)) printf("Failed to add N-gram into trie!\n");
	}

	/*Queue queue;
	QueueCreate(&queue);*/

	while((n = getline(&buffer,&buffer_size,query)) > 0){
		if(buffer[n-1] == '\n')
			buffer[n-1] = '\0';

		n--;

		if(buffer[0] == 'A' && isspace(buffer[1])){
			/*We have to add an N_gram*/
			if(TrieAdd(trie,&buffer[2])){
				#ifdef DEBUG
				printf("Error in add!\n");
				#endif
			}
		}
		else if(buffer[0] == 'D' && isspace(buffer[1])){
			/*We have to delete an N_gram*/
			if(TrieDelete(trie,&buffer[2])){
				#ifdef DEBUG
				printf("Error in delete!\n");
				#endif
			}
		}
		else if(buffer[0] == 'Q' && isspace(buffer[1])){
			/* We have to delete an N_gram */
			Queue result = TrieQuestion(trie,&buffer[2]);
			if(result == NULL){
				printf("-1\n");
				continue;
			}

			char* s = (char*) QueuePop(result);
			printf("%s", s);
			free(s);
			while( (s=(char*)QueuePop(result)) ){
				printf("|%s", s);
				free(s);
			}
			printf("\n");

			QueueDestroy(&result);
		}
		else if(buffer[0] == 'F'){
			/*for(i = 1;i < n;i++){
				if(!isspace(buffer[i])) break;
			}

			if(i < n){
				printf("Error in F\n");
				continue;
			}

			printf("F should work here...\n");
			*/
		}
		else printf("Wrong Command!\n");
	}

	/*Destroy any remnants in the queue if no F was given at the end of the file*/
	/*while((s = QueuePop(queue))) free(s);
	QueueDestroy(&queue);*/
	TrieDestroy(&trie);


	free(buffer);
	fclose(init);
	fclose(query);

	return 0;
}