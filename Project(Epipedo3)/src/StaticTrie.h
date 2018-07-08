#ifndef 	__STATICTRIE_H__
#define 	__STATICTRIE_H__

#include "Queue.h"
#include "BloomFilter.h"
/*

Project team:
1) Stefou Theodoros 		1115201400193
2) Kokkinakos Panagiotis	1115201400069
3) Rouvalis Georgios		1115201400173

*/

#define LINEAR_HASH_M 1024
#define LINEAR_HASH_C 10

typedef char Bool;

typedef struct HyperNode
{
	char* word;
	struct HyperNode* children;
	int arraySize;
	int numChildren;
	short* lengths;
	int lengths_size;
}HyperNode;

struct StaticTrie_t
{
	HyperNode* root;
	int initial_size;
};

typedef struct StaticTrie_t* StaticTrie;

Bool		StaticTrieCreate(void** trie, int initial_size);
Bool		StaticTrieDestroy(void** trie);
void*	StaticTrieQuestion(void* args);
Bool		StaticTrieAdd(void* trieptr,char* N_gram,int id);
Bool 	StaticTrieAddForbid(void* trieptr,char* N_gram,int id);
Bool		StaticTrieDelete(void* trieptr,char* N_gram,int id);
Bool 	StaticTrieCompress(StaticTrie trie);
Bool 	StaticTrieCleanup(void* trie);
void		StaticTrieSizeStatistics(void* trie, int* bytes, int* nodes);

#endif
