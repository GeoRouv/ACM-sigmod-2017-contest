#ifndef 	__ASSISTINGFUNCTIONS_H__
#define 	__ASSISTINGFUNCTIONS_H__

#include "BloomFilter.h"
#include "Queue.h"

typedef char Bool;

typedef struct ngram_frequency{
	char* ngram;
	int times;
}ngram_freq;

/*Arguments for the question function*/
typedef struct q_args{
	void* trieptr;
	char* string;
	BloomFilter bf;
	Queue* burst;
	int id;
	int q_id;

}q_args;

Bool isFlag(char* word);
void print_usage(char* prog_name);
void MapInsert(char* ngram,ngram_freq** array,int* size,int* entries);
char** break_string(char* s, int* words);
char* GetFirstToken(char* sentence, int* next_offset);
int mypow(int x, int y);

#endif
