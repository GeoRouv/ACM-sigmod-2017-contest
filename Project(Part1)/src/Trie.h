#ifndef 	__TRIE_H__
#define 	__TRIE_H__

#include "Queue.h"
/*

Project team:
1) Stefou Theodoros 		1115201400193
2) Kokkinakos Panagiotis	1115201400069
3) Rouvalis Georgios		1115201400173

*/

typedef char Bool;

typedef struct Trie_t* Trie;

Bool	TrieCreate(Trie* trie, int initial_size);
Bool	TrieDestroy(Trie* trie);
Queue	TrieQuestion(Trie trie, char* sentence);
Bool	TrieAdd(Trie trie, char* N_gram);
Bool	TrieDelete(Trie trie, char* N_gram);

#endif
