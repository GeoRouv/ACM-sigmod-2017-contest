#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "DynamicTrie.h"
#include "Queue.h"
#include "BloomFilter.h"
#include "LinearHash.h"
#include "AssistingFunctions.h"

/*

Project team:
1) Stefou Theodoros 		1115201400193
2) Kokkinakos Panagiotis	1115201400069
3) Rouvalis Georgios		1115201400173

*/

/* Every Bool function returns 0 on success, 1 on failure */

Bool deleted;

Bool TrieNodeCreate(TrieNode* n, char* word, int arraySize)
{
	/*Initialize a new node for trie insertion*/
	n->word = malloc((strlen(word)+1)*sizeof(char));
	if(n->word == NULL) return 1;
	strcpy(n->word, word);
	
	n->children = malloc(arraySize*sizeof(TrieNode));
	if(n->children == NULL) return 1;
	
	n->arraySize = arraySize;
	n->numChildren = 0;
	n->isFinal = 0;

	return 0;
}

Bool DynamicTrieCreate(void** trieptr, int initial_size)
{
	Trie* trie = (Trie*)trieptr;
	/*Create and initialize a trie*/
	(*trie) = malloc(sizeof(struct Trie_t));
	if(!(*trie)) return 1;

	(*trie)->initial_size = initial_size;
	(*trie)->root = malloc(sizeof(TrieNode));
	(*trie)->root->word = NULL;

	LinearHash L; LinearHashCreate(&L, LINEAR_HASH_M,LINEAR_HASH_C);
	(*trie)->root->children = (TrieNode*) L; /* This is actually a pointer to a Linear HashTable. We will cast this later when we need it. */

	BloomFilterCreate( &((*trie)->bf) );

	return 0;
}

Bool DynamicTrieAdd(void* trieptr, char* N_gram)
{
	Trie trie = (Trie)trieptr;
	/*Adds a given n-gram in the trie*/
	int i,comp;

	for(i = 0;i < strlen(N_gram);i++){
		if(!isspace(N_gram[i])) break;
	}

	if(i == strlen(N_gram)) return 1;

	int first,last,middle;

	char* token = strtok(N_gram, " \t");
	int x,y;
	TrieNode* tmp = LinearHashLookup((LinearHash)(trie->root->children), token, &x, &y);

	TrieNode toInsert; /* At the end, remember the node we need to insert. */
	Bool insert_at_end = 0;
	if(tmp == NULL)
	{
		insert_at_end = 1;
		tmp = &toInsert;
		toInsert.word = malloc((strlen(token)+1)*sizeof(char));
		strcpy(toInsert.word, token);
		toInsert.children = malloc(trie->initial_size*sizeof(TrieNode));
		toInsert.arraySize = trie->initial_size;
		toInsert.numChildren = 0;
		toInsert.isFinal = 0;
	}
	
	token = strtok(NULL, " \t");
	Bool found = 0;
	while(token){

		first = 0;
		last =  tmp->numChildren-1;
		middle = (first+last)/2;

		while(first <= last){ /*Binary search*/
			comp = strcmp(token,tmp->children[middle].word);
			if(comp > 0)
				first = middle + 1;
			else if(!comp){
				found = 1;
				break;
			}
			else 
				last = middle -1;
			middle = (first+last)/2;
		}

		if(first > last) found = 0; /*Child not found*/

		if(found) /* Word already in path */
			tmp = &(tmp->children[middle]);
		else{ /* Word not found in next node */
			if(tmp->arraySize == tmp->numChildren){
				/* Array full. Must realloc */
				tmp->children = realloc(tmp->children, 2*(tmp->arraySize)*sizeof(TrieNode));
				if(!tmp->children) return 1;
				tmp->arraySize = 2*(tmp->arraySize);
			}
			TrieNode n;
			if(TrieNodeCreate(&n, token, trie->initial_size)) return 1;

			memmove(tmp->children+first+1, tmp->children+first, (tmp->numChildren-first)*sizeof(TrieNode));
			tmp->children[first] = n;
			tmp->numChildren++;
			tmp = &(tmp->children[first]);
		}

    	token = strtok(NULL, " \t");
    	found = 0;
	}

	tmp->isFinal = 1;

	if(insert_at_end)
		LinearHashInsert((LinearHash)(trie->root->children), toInsert);

	return 0;
}

Queue DynamicTrieQuestion(void* trieptr, char* sentence)
{
	Trie trie = (Trie)trieptr;
	/*Returns every n-gram that exists in the trie and is a sub-string of the given sentence*/
	int words,i,j,curr_word_ind,result_size,comp;
	Queue result;

	QueueCreate(&result);

	for(i = 0;i < strlen(sentence);i++){
		if(!isspace(sentence[i])) break;
	}

	if(i == strlen(sentence)){
		char* mone = malloc(sizeof("-1"));
		strcpy(mone,"-1");
		QueuePush(result, (void*)mone);
		return result;
	}

	char** split_array = break_string(sentence, &words);

	int first,last,middle;
	result_size = 0;

	/*Parse the words of the given sentence*/
	for(i = 0;i < words;i++){
		curr_word_ind = i;
		int x,y;
		TrieNode* tmp = LinearHashLookup((LinearHash)(trie->root->children), split_array[curr_word_ind], &x, &y);
		if(tmp == NULL) continue;
		if(tmp->isFinal){ /*If we are in a final word, copy the phrase into the result queue*/
			int chars = 0;
			for(j = i; j <= curr_word_ind; j++)
				chars += strlen(split_array[j])+1;
			
			char* s = malloc(chars*sizeof(char));
			sprintf(s, "%s", split_array[i]);

			if( BloomFilterAsk(trie->bf,s) )
				QueuePush(result, (void*) s);
			else free(s);
			result_size++;
		}
		curr_word_ind++;

		while(curr_word_ind < words){
			first = 0;
			last =  tmp->numChildren-1;
			middle = (first+last)/2;
			
			while(first <= last){ /*Binary search*/
				comp = strcmp(split_array[curr_word_ind],tmp->children[middle].word);
				if(comp > 0)
					first = middle + 1;
				else if(!comp){
					break;
				}
				else 
					last = middle -1;
				middle = (first+last)/2;
			}

			if(first > last) break; /*Child not found*/

			tmp = &(tmp->children[middle]);

			if(tmp->isFinal){ /*If we are in a final word, copy the phrase into the result queue*/
				int chars = 0;
				for(j = i; j <= curr_word_ind; j++)
					chars += strlen(split_array[j])+1;
				
				char* s = malloc(chars*sizeof(char));
				sprintf(s, "%s", split_array[i]);
				for(j = i+1; j <= curr_word_ind; j++)
					sprintf(s+strlen(s), " %s", split_array[j]);

				if( BloomFilterAsk(trie->bf,s) )
					QueuePush(result, (void*) s);
				else free(s);
				result_size++;
			}

			curr_word_ind++;
		}
	}

	free(split_array);
	BloomFilterReset(trie->bf);

	/* If no results found, return -1 */
	if(result_size == 0){
		char* mone = malloc(sizeof("-1"));
		strcpy(mone,"-1");
		QueuePush(result, (void*)mone);
	}

	return result; 
}

void DynamicTrieDestroyPrivate(TrieNode* node)
{
	int i;

	for(i = 0;i < node->numChildren;i++)
		DynamicTrieDestroyPrivate(&(node->children[i]));
	free(node->word);
	free(node->children);
}

Bool DynamicTrieDestroy(void** trieptr)
{
	Trie* trie = (Trie*)trieptr;
	/*Completely destroy a trie*/

	BloomFilterDestroy(&((*trie)->bf));

	LinearHash L = (LinearHash)((*trie)->root->children);

	int i,j;
	for(i = 0; i < L->curr_m; i++)
		for(j = 0; j < L->buckets[i].arr_entries; j++)
			DynamicTrieDestroyPrivate(&(L->buckets[i].array[j]));

	LinearHashDestroy(&L);

	free((*trie)->root);
	free(*trie);

	*trie = NULL;

	return 0;
}

Bool DynamicTrieDeletePrivate(TrieNode* node, char* N_gram)
{
	int next_token;
	char* first = GetFirstToken(N_gram, &next_token);
	int i;
	for(i = 0; i < node->numChildren; i++)
		if(!strcmp(node->children[i].word, first)) break; //Binary
	free(first);
	
	int found = i < node->numChildren;
	if(found && next_token > 0){ /*There are more words to be parsed in the given string*/
		int ret = DynamicTrieDeletePrivate(&(node->children[i]), &N_gram[next_token]);
		if(ret == 0){
			memmove(node->children+i, node->children+i+1, (node->numChildren-i-1)*sizeof(TrieNode));
			node->numChildren--;
			if(node->numChildren > 0 || node->isFinal)
				return 1;
			else{
				free(node->word);
				free(node->children);
				return 0;
			}
		}
		else return 1;
	}
	else if(found && next_token == -1){
		if(node->children[i].numChildren == 0 && node->children[i].isFinal){
			deleted = 0;
			free(node->children[i].word);
			free(node->children[i].children);
			memmove(node->children+i, node->children+i+1, (node->numChildren-i-1)*sizeof(TrieNode));
			node->numChildren--;
			if(node->numChildren == 0 && !node->isFinal){
				free(node->word);
				free(node->children);
				return 0;
			}
			else return 1;
		}
		else if(node->children[i].numChildren > 0 && node->children[i].isFinal){
			deleted = 0;
			node->children[i].isFinal = 0;
			return 1;
		}
		else return 1;
	}
	else if(!found) return 1;
	
	return 1;
}

Bool DynamicTrieDelete(void* trieptr, char* N_gram)
{
	Trie trie = (Trie)trieptr;

	/* Delete a given n-gram using TrieDeletePrivate() recursively */
	int i=0,j;

	while(isspace(N_gram[i])) i++;

	if(N_gram[i] == '\0') return 1; /* It is an empty string. */

	int chars = 0;
	j = i;
	while(!isspace(N_gram[j]) && N_gram[j] != '\0'){
		chars++;
		j++;
	}
	char* s = malloc((chars+1)*sizeof(char));
	sscanf(N_gram+i, "%s", s);

	int bucket,position;
	TrieNode* tmp = LinearHashLookup((LinearHash)(trie->root->children), s, &bucket, &position);

	deleted = 1;

	if(tmp == NULL){
		free(s);
		return deleted;
	}

	while(isspace(N_gram[j])) j++;

	if(tmp->numChildren == 0 && N_gram[j] == '\0'){ /* Is a final node and only one word was given. */
		LinearHashDeleteAt((LinearHash)(trie->root->children), bucket, position, 1);
		free(s);
		return 0;
	}
	else if(N_gram[j] == '\0'){ /* Only one word was given. */
		tmp->isFinal = 0;
		free(s);
		return 0;
	}


	if(DynamicTrieDeletePrivate(tmp, N_gram+j) == 0)
		LinearHashDeleteAt((LinearHash)(trie->root->children), bucket, position, 0);
	
	free(s);

	return deleted;
}
