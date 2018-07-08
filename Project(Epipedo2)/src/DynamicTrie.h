#ifndef 	__DYNAMICTRIE_H__
#define 	__DYNAMICTRIE_H__

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

typedef struct TrieNode
{
	char* word;
	struct TrieNode* children;
	int arraySize;
	int numChildren;
	Bool isFinal;
}TrieNode;

struct Trie_t
{
	TrieNode* root;
	int initial_size;
	BloomFilter bf;
};

typedef struct Trie_t* Trie;

Bool	DynamicTrieCreate(void** trieptr, int initial_size);
Bool	DynamicTrieDestroy(void** trieptr);
Queue	DynamicTrieQuestion(void* trieptr, char* sentence);
Bool	DynamicTrieAdd(void* trieptr, char* N_gram);
Bool	DynamicTrieDelete(void* trie, char* N_gram);

#endif
