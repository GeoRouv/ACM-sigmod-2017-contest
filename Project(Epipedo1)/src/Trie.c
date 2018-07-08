#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "Trie.h"
#include "Queue.h"

/*

Project team:
1) Stefou Theodoros 		1115201400193
2) Kokkinakos Panagiotis	1115201400069
3) Rouvalis Georgios		1115201400173

*/

/* Every Bool function returns 0 on success, 1 on failure */

Bool deleted;

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
};

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

Bool TrieCreate(Trie* trie, int initial_size)
{
	/*Create and initialize a trie*/
	(*trie) = malloc(sizeof(struct Trie_t));
	if(!(*trie)) return 1;

	(*trie)->initial_size = initial_size;
	(*trie)->root = malloc(sizeof(TrieNode));
	(*trie)->root->word = NULL;
	(*trie)->root->children = malloc((*trie)->initial_size*sizeof(TrieNode));
	(*trie)->root->arraySize = (*trie)->initial_size;
	(*trie)->root->numChildren = 0;
	(*trie)->root->isFinal = 0;

	return 0;
}

/*void shift_right(TrieNode** array, int numChildren, int n)
{
	int i;
	for(i = numChildren-1; i >= n; i--)
		array[i+1] = array[i];
}*/

Bool TrieAdd(Trie trie, char* N_gram)
{
	/*Adds a given n-gram in the trie*/
	int i,comp;

	for(i = 0;i < strlen(N_gram);i++){
		if(!isspace(N_gram[i])) break;
	}

	if(i == strlen(N_gram)) return 1;

	int first,last,middle;

	char* token = strtok(N_gram, " \t");
	TrieNode* tmp = trie->root;
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

	return 0;
}

char** break_string(char* s, int* words)
{
	/* Returns a 2D array, each line pointing to a word from s */
	char** ret;
	int i = 0;
	*words = 0;
	int len = strlen(s); /*Compiler might optimize this? */

	while(s[i] == ' ' || s[i] == '\t') i++;

	for( ; i < len; i++){
		if(s[i] == ' ' || s[i] == '\t'){
			(*words)++;
			while(s[i] == ' ' || s[i] == '\t') i++;
		}
	}
	if(s[len-1] != ' ' && s[len-1] != '\t') (*words)++;

	ret = malloc((*words)*sizeof(char*));
	char* token = strtok(s, " \t");
	i = 0;
	while(token){
		ret[i++] = token;
		token = strtok(NULL, " \t");
	}

	return ret;
}

Queue TrieQuestion(Trie trie, char* sentence)
{
	/*Returns every n-gram that exists in the trie and is a sub-string of the given sentence*/
	int words,i,j,curr_word_ind,result_size;
	Queue result;

	for(i = 0;i < strlen(sentence);i++){
		if(!isspace(sentence[i])) break;
	}

	if(i == strlen(sentence)) return NULL;

	QueueCreate(&result);

	char** split_array = break_string(sentence, &words);

	int first,last,middle;
	result_size = 0;
	/*Parse the words of the given sentence*/
	for(i = 0;i < words;i++){
		curr_word_ind = i;
		TrieNode* tmp = trie->root;

		while(curr_word_ind < words){
			first = 0;
			last =  tmp->numChildren-1;
			middle = (first+last)/2;
			
			while(first <= last){ /*Binary search*/
				if(strcmp(split_array[curr_word_ind],tmp->children[middle].word) > 0)
					first = middle + 1;
				else if(!strcmp(split_array[curr_word_ind],tmp->children[middle].word)){
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
				QueuePushUnique(result, (void*) s);
				result_size++;
			}

			curr_word_ind++;
		}
	}

	free(split_array);

	/* If no results found, return NULL and main will print -1 */
	if(result_size == 0){
		QueueDestroy(&result);
		return NULL;
	}

	return result; 
}

void TrieDestroyPrivate(TrieNode* node)
{
	int i;

	for(i = 0;i < node->numChildren;i++)
		TrieDestroyPrivate(&(node->children[i]));
	free(node->word);
	free(node->children);
}

Bool TrieDestroy(Trie* trie)
{
	/*Completely destroy a trie*/
	int i;
	for(i = 0;i < (*trie)->root->numChildren;i++)
		TrieDestroyPrivate(&((*trie)->root->children[i]));

	free((*trie)->root->children);
	free((*trie)->root);
	free(*trie);

	*trie = NULL;

	return 0;
}

char* GetFirstToken(char* sentence, int* next_offset)
{
	/*Returns the first token of a given string and the offset of the next token, if it exists*/
    int i, pos1,pos2;
	i = 0;
	while(isspace(sentence[i])) i++;
	pos1 = i;
	while(sentence[i] != '\0' && !isspace(sentence[i])) i++;
	pos2 = i-1;
	char* ret = malloc((pos2-pos1+2)*sizeof(char));
	for(i = pos1; i <= pos2; i++)
		ret[i-pos1] = sentence[i];
	ret[i] = '\0';
	
	/* Calculate offset for next word */
	pos2++;
	while(sentence[pos2] != '\0' && isspace(sentence[pos2])) pos2++;
	if(sentence[pos2] == '\0') *next_offset = -1;
	else *next_offset = pos2;
	
	return ret;
}

/*void shift_left(TrieNode** array, int numChildren, int n)
{
	if(n < numChildren-1){
		int i;
		for(i = n; i < numChildren-1; i++) array[i] = array[i+1];
	}
}*/

Bool TrieDeletePrivate(TrieNode* node, char* N_gram)
{
	int next_token;
	char* first = GetFirstToken(N_gram, &next_token);
	int i;
	for(i = 0; i < node->numChildren; i++)
		if(!strcmp(node->children[i].word, first)) break;
	free(first);
	
	int found = i < node->numChildren;
	if(found && next_token > 0){ /*There are more words to be parsed in the given string*/
		int ret = TrieDeletePrivate(&(node->children[i]), &N_gram[next_token]);
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

Bool TrieDelete(Trie trie, char* N_gram)
{
	/*Delete a given n-gram using TrieDeletePrivate() recursively*/
	int i;

	for(i = 0; i < strlen(N_gram); i++){
		if(!isspace(N_gram[i])) break;
	}

	if(i == strlen(N_gram)) return 1;

	deleted = 1;

	TrieDeletePrivate(trie->root, N_gram);

	return deleted;
}