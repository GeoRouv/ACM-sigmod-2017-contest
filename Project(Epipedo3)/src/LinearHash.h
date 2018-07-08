#ifndef __LINEARHASH_H__
#define __LINEARHASH_H__

#include "DynamicTrie.h"

typedef struct LinearHash_t* LinearHash;

typedef struct Bucket
{
	TrieNode* array;
	int arr_size;
	int arr_entries;

} Bucket;

typedef struct LinearHash_t
{

	Bucket* buckets;
	int m; 		/* Initial number of buckets */
	int curr_m; /* Current number of buckets */
	int c; 		/* Initial number of cells in each bucket */
	int p; 		/* Next bucket to be split */
	int round;

} LinearHash_t;

typedef char Bool;

void 		LinearHashCreate(LinearHash* L, unsigned int m, unsigned int c);
Bool 		LinearHashInsert(LinearHash L, TrieNode n);
TrieNode* 	LinearHashLookup(LinearHash L, char* key, int* bucket_found, int* pos_found);
Bool		LinearHashDeleteAt(LinearHash L, int bucket_found, int pos_found, int delete);
void 		LinearHashDestroy(LinearHash* L);

#endif 
