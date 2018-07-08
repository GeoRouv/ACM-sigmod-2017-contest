#ifndef __STATICLINEARHASH_H__
#define __STATICLINEARHASH_H__

#include "StaticTrie.h"

typedef struct StaticLinearHash_t* StaticLinearHash;

typedef struct StaticBucket
{
	HyperNode* array;
	int arr_size;
	int arr_entries;

} StaticBucket;

typedef struct StaticLinearHash_t
{

	StaticBucket* buckets;
	int m; 		/* Initial number of buckets */
	int curr_m; /* Current number of buckets */
	int c; 		/* Initial number of cells in each bucket */
	int p; 		/* Next bucket to be split */
	int round;

} StaticLinearHash_t;

typedef char Bool;

void 				StaticLinearHashCreate(StaticLinearHash* L, unsigned int m, unsigned int c);
Bool 				StaticLinearHashInsert(StaticLinearHash L, HyperNode n);
HyperNode* 	StaticLinearHashLookup(StaticLinearHash L, char* key, int* bucket_found, int* pos_found);
void 				StaticLinearHashDestroy(StaticLinearHash* L);

#endif 
