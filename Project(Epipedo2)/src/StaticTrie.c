#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "StaticTrie.h"
#include "Queue.h"
#include "BloomFilter.h"
#include "StaticLinearHash.h"
#include "AssistingFunctions.h"

/*

Project team:
1) Stefou Theodoros 		1115201400193
2) Kokkinakos Panagiotis	1115201400069
3) Rouvalis Georgios		1115201400173

*/

/* Every Bool function returns 0 on success, 1 on failure */

Bool compressed = 0;

Bool HyperNodeCreate(HyperNode* n, char* word, int arraySize)
{
	/*Initialize a new node for trie insertion*/
	n->word = malloc((strlen(word)+1)*sizeof(char));
	if(n->word == NULL) return 1;
	strcpy(n->word, word);
	
	n->children = malloc(arraySize*sizeof(HyperNode));
	if(n->children == NULL) return 1;
	
	n->arraySize = arraySize;
	n->numChildren = 0;
	n->lengths = malloc(sizeof(short));
	n->lengths[0] = -strlen(n->word);
	n->lengths_size = 1;

	return 0;
}

Bool StaticTrieCreate(void** trieptr, int initial_size)
{
	StaticTrie* trie = (StaticTrie*)trieptr;
	/*Create and initialize a trie*/
	(*trie) = malloc(sizeof(struct StaticTrie_t));
	if(!(*trie)) return 1;

	(*trie)->initial_size = initial_size;
	(*trie)->root = malloc(sizeof(HyperNode));
	(*trie)->root->word = NULL;

	StaticLinearHash L; StaticLinearHashCreate(&L, LINEAR_HASH_M,LINEAR_HASH_C);
	(*trie)->root->children = (HyperNode*) L; /* This is actually a pointer to a Linear HashTable. We will cast this later when we need it. */

	BloomFilterCreate( &((*trie)->bf) );

	return 0;
}

Bool StaticTrieAdd(void* trieptr, char* N_gram)
{
	/* This should only be called at the beginning of the program to initialize. */
	/*Adds a given n-gram in the trie*/
	StaticTrie trie = (StaticTrie)trieptr;
	int i,comp;

	for(i = 0;i < strlen(N_gram);i++)
		if(!isspace(N_gram[i])) break;

	if(i == strlen(N_gram)) return 1;

	int first,last,middle;

	char* token = strtok(N_gram, " \t");
	int x,y;
	HyperNode* tmp = StaticLinearHashLookup((StaticLinearHash)(trie->root->children), token, &x, &y);

	HyperNode toInsert; /* At the end, remember the node we need to insert. */
	Bool insert_at_end = 0;
	if(tmp == NULL)
	{
		insert_at_end = 1;
		tmp = &toInsert;
		if(HyperNodeCreate(tmp, token, trie->initial_size)) return 1;
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
				tmp->children = realloc(tmp->children, 2*(tmp->arraySize)*sizeof(HyperNode));
				if(!tmp->children) return 1;
				tmp->arraySize = 2*(tmp->arraySize);
			}
			HyperNode n;
			if(HyperNodeCreate(&n, token, trie->initial_size)) return 1;

			memmove(tmp->children+first+1, tmp->children+first, (tmp->numChildren-first)*sizeof(HyperNode));
			tmp->children[first] = n;
			tmp->numChildren++;
			tmp = &(tmp->children[first]);
		}

    	token = strtok(NULL, " \t");
    	found = 0;
	}

	/*The final Hypercube's word is final*/
	tmp->lengths[0] *= -1;

	if(insert_at_end)
		StaticLinearHashInsert((StaticLinearHash)(trie->root->children), toInsert);

	return 0;
}

Queue StaticTrieQuestion(void* trieptr, char* sentence)
{
	/*Returns every n-gram that exists in the trie and is a sub-string of the given sentence*/
	StaticTrie trie = (StaticTrie)trieptr;
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
	int cut;
	/*Parse the words of the given sentence*/
	for(i = 0;i < words;i++){
		cut = 0;
		curr_word_ind = i;
		int x,y;

		HyperNode* tmp = StaticLinearHashLookup((StaticLinearHash)(trie->root->children), split_array[curr_word_ind], &x, &y);
		if(tmp == NULL) continue;

		if(tmp->lengths[0] > 0){ /*If the 1st word of the node is final copy it in the result*/
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

		int len = 1; /*Which word in the hypernode you are checking*/
		int index = abs(tmp->lengths[0]);
		curr_word_ind++;
		/* Go down the words of the 1st hyper node, while there is a match between them and the input n-gram */
		while((len < tmp->lengths_size) && (curr_word_ind < words)){
			int count;
			int absol = abs(tmp->lengths[len]);
			
			/*if(strlen(split_array[curr_word_ind]) < absol)
				count = absol;
			else count = strlen(split_array[curr_word_ind]);

			comp = strncmp(split_array[curr_word_ind],tmp->word+index,count);*/

			comp = strncmp(split_array[curr_word_ind],tmp->word+index,absol);
			if(absol != strlen(split_array[curr_word_ind])){
				if(comp == 0){
					if(absol < strlen(split_array[curr_word_ind])) comp = 1;
					else comp = -1;
				}	
			}

			if(comp){
				cut = 1;
				break;
			}

			if(tmp->lengths[len] > 0){ /*If we are in a final word, copy the phrase into the result queue*/
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

			index += absol;
			len++;
			curr_word_ind++;
		}

		
		if(cut) continue;

		cut = 0;
		
		while(curr_word_ind < words){
			first = 0;
			last =  tmp->numChildren-1;
			middle = (first+last)/2;			
			
			/*Binary search-Find if there is any hyper node which begins with the next word in the input n-gram*/
			while(first <= last){
				
				int count;
				int absol = abs(tmp->children[middle].lengths[0]);
				/*if(strlen(split_array[curr_word_ind]) < absol)
					count = absol;
				else count = strlen(split_array[curr_word_ind]);*/

				comp = strncmp(split_array[curr_word_ind],tmp->children[middle].word,absol);
				if(absol != strlen(split_array[curr_word_ind])){
					if(comp == 0){
						if(absol < strlen(split_array[curr_word_ind])) comp = 1;
						else comp = -1;
					}	
				}
					
				/*comp = strncmp(split_array[curr_word_ind],tmp->children[middle].word,count);*/
				
				if(comp > 0)
					first = middle + 1;
				else if(!comp)
					break;
				else 
					last = middle -1;
				middle = (first+last)/2;
			}

			if(first > last) break; /*Child not found*/

			tmp = &(tmp->children[middle]);
			
			int len = 0;
			int index = 0;
			/*Go down the words of the hypernode we found previously, while they match the input n-gram*/
			while((len < tmp->lengths_size) && (curr_word_ind < words)){

				int count;
				int absol = abs(tmp->lengths[len]);
				
				/*if(strlen(split_array[curr_word_ind]) < absol)
					count = absol;
				else count = strlen(split_array[curr_word_ind]);

				comp = strncmp(split_array[curr_word_ind],tmp->word+index,count);*/

				comp = strncmp(split_array[curr_word_ind],tmp->word+index,absol);
				if(absol != strlen(split_array[curr_word_ind])){
					if(comp == 0){
						if(absol < strlen(split_array[curr_word_ind])) comp = 1;
						else comp = -1;
					}	
				}

				if(comp){
					cut = 1;
					break;
				}

				if(tmp->lengths[len] > 0){ /*If we are in a final word, copy the phrase into the result queue*/
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

				index += absol;
				len++;
				curr_word_ind++;
			}
			
			if(cut) break;
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

void StaticTrieDestroyPrivate(HyperNode* node)
{
	int i;

	for(i = 0;i < node->numChildren;i++)
		StaticTrieDestroyPrivate(&(node->children[i]));
	free(node->word);
	free(node->lengths);
	if((node->numChildren > 0 && compressed) || (!compressed))
		free(node->children);
}

Bool StaticTrieDestroy(void** trieptr)
{
	/*Completely destroy a trie*/
	StaticTrie* trie = (StaticTrie*)trieptr;
	BloomFilterDestroy(&((*trie)->bf));

	StaticLinearHash L = (StaticLinearHash)((*trie)->root->children);

	int i,j;
	for(i = 0; i < L->curr_m; i++)
		for(j = 0; j < L->buckets[i].arr_entries; j++)
			StaticTrieDestroyPrivate(&(L->buckets[i].array[j]));

	StaticLinearHashDestroy(&L);

	free((*trie)->root);
	free(*trie);

	*trie = NULL;

	return 0;
}

Bool StaticTrieCompressPrivate(HyperNode* hnode)
{
	int i;
	int word_size;

	HyperNode* tmp = hnode;
	HyperNode* to_destroy;
	Queue del;

	word_size = strlen(hnode->word)+1;

	if(tmp->numChildren == 1) /*If the 1st node has 1 children, move to the next and compress*/
		tmp = &(tmp->children[0]);
	else if(tmp->numChildren > 1){ /*If the 1st node has 2 or more children, call the compress function for each one of them*/
		for(i = 0; i < tmp->numChildren;i++)
			if(tmp->children[i].numChildren > 0)
				StaticTrieCompressPrivate(&(tmp->children[i]));
			else free(tmp->children[i].children);
		return 0;
	}

	QueueCreate(&del);

	while(tmp->numChildren == 1){ /*Move down until a node with 0 or 2 or more children is found*/
		word_size += strlen(tmp->word);
		hnode->word = realloc(hnode->word,word_size*sizeof(char));
		sprintf(hnode->word+strlen(hnode->word),"%s",tmp->word);
		
		hnode->lengths = realloc(hnode->lengths,(hnode->lengths_size+1)*sizeof(short));
		hnode->lengths[hnode->lengths_size] = tmp->lengths[0];
		hnode->lengths_size++;

		to_destroy = tmp;
		tmp = &(tmp->children[0]);

		free(to_destroy->word);
		QueuePush(del,to_destroy->children);
		free(to_destroy->lengths);
	}

	word_size += strlen(tmp->word);
	hnode->word = realloc(hnode->word,word_size*sizeof(char));
	sprintf(hnode->word+strlen(hnode->word),"%s",tmp->word);
	
	hnode->lengths = realloc(hnode->lengths,(hnode->lengths_size+1)*sizeof(short));
	hnode->lengths[hnode->lengths_size] = tmp->lengths[0];
	hnode->lengths_size++;

	free(tmp->word);
	free(tmp->lengths);

	if(tmp->numChildren == 0){ /*If the last node has 0 children, compress it and destroy it*/

		free(tmp->children);
		hnode->numChildren = 0;
		QueuePush(del,hnode->children);
	}
	else if(tmp->numChildren > 1){ /*If it has 2 or more, compress it and run the compress for its children*/

		hnode->numChildren = tmp->numChildren;
    
    	HyperNode* swap = malloc(hnode->numChildren*sizeof(HyperNode));
    
		memmove(swap,tmp->children,hnode->numChildren*sizeof(HyperNode));
		free(tmp->children);	
    	free(hnode->children);
		hnode->children = swap;		
		
		for (i = 0; i < hnode->numChildren;i++)
			if(hnode->children[i].numChildren > 0)
				StaticTrieCompressPrivate(&(hnode->children[i]));
			else{
				free(hnode->children[i].children);
				hnode->children[i].numChildren = 0;
			}
	}

	void * delete;

	while((delete = QueuePop(del))) free(delete);
	QueueDestroy(&del);

	return 0;
}

Bool StaticTrieCompress(StaticTrie trie)
{
	int i,j;

	StaticLinearHash L = (StaticLinearHash)(trie->root->children);
	for(i = 0; i < L->curr_m;i++)
		for(j = 0; j < L->buckets[i].arr_entries; j++)
			/*If the bucket node has children then perform the compress, else dont do anything */
			if(L->buckets[i].array[j].numChildren > 0) 
				StaticTrieCompressPrivate(&(L->buckets[i].array[j]));
			else
				free(L->buckets[i].array[j].children);
	compressed	= 1;

	return 0;
} 

Bool StaticTrieAddForbid(void* trieptr, char* N_gram)
{
	printf("Requested action is forbidden in static tries\n");

	return 1;
}

Bool StaticTrieDelete(void* trieptr, char* N_gram)
{
	printf("Requested action is forbidden in static tries\n");

	return 1;
}


