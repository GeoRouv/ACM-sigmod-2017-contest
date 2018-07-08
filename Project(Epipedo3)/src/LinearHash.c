#include <stdlib.h>
#include <string.h>

#include "DynamicTrie.h"
#include "LinearHash.h"
#include "AssistingFunctions.h"

#include "MurmurHash2.h"

#define MURMUR_SEED 941762


int HashesAt(LinearHash L, char* key, int i)
{
	return MurmurHash2( key, strlen(key), MURMUR_SEED) % ( mypow(2,i)*(L->m) );
}

void LinearHashCreate(LinearHash* L, unsigned int m, unsigned int c)
{
	int i;

	(*L) = malloc(sizeof(LinearHash_t));
	(*L)->m = m;
	(*L)->curr_m = m;
	(*L)->c = c;
	(*L)->p = 0;
	(*L)->round = 0;
	(*L)->buckets = malloc(m*sizeof(Bucket));

	for(i = 0; i < m; i++)
	{
		(*L)->buckets[i].array = malloc(c*sizeof(TrieNode));
		(*L)->buckets[i].arr_size = c;
		(*L)->buckets[i].arr_entries = 0;
	}
}

Bool LinearHashInsert(LinearHash L, TrieNode n)
{
	/* split remembers if there was an overflow,
	and if there was, perform the split */
	int first, middle, last, comp,i, split = 0;

	int hash = HashesAt(L, n.word, L->round);
	if(hash < L->p)
		hash = HashesAt(L, n.word, L->round+1);

	Bucket* bucket = &(L->buckets[hash]);

	first = 0;
	last =  bucket->arr_entries-1;
	middle = (first+last)/2;
	
	while(first <= last){ /*Binary search*/
		comp = strcmp(n.word, bucket->array[middle].word);
		if(comp > 0)
			first = middle + 1;
		else if(!comp)
			break;
		else 
			last = middle-1;
		
		middle = (first+last)/2;
	}

	if(first <= last) return 0; /* Found */

	/* Else not found */
	if(bucket->arr_size == bucket->arr_entries)
	{
 		/* Double the size of the overflown array */   
		bucket->array = realloc(bucket->array, 2*bucket->arr_size*sizeof(TrieNode));
		bucket->arr_size *= 2;
		split = 1;
	}
 			
    memmove(bucket->array+first+1, bucket->array+first,(bucket->arr_entries-first)*sizeof(TrieNode));
    bucket->arr_entries++;
    bucket->array[first] = n;
	  
	if(split){  
	    /* Perform the split */
	    L->curr_m++;
	    
	    L->buckets = realloc(L->buckets,(L->curr_m)*sizeof(Bucket));
		
		/* The new bucket will have the same size as the one to be split.
		That is so that there will be no chain reaction of splits. */
	    L->buckets[L->curr_m-1].array = malloc(L->buckets[L->p].arr_size*sizeof(TrieNode));
	    L->buckets[L->curr_m-1].arr_size = L->buckets[L->p].arr_size;
	    L->buckets[L->curr_m-1].arr_entries = 0;

		int hashes_at;
	    int index = 0; /* This index points to the next element that we will check if it needs redistribution. */
		while(index < L->buckets[L->p].arr_entries){
	      hashes_at = HashesAt(L, L->buckets[L->p].array[index].word, L->round+1);
	      if(hashes_at == L->p){ /* If hashes on the same, leave it there, else move to the newly formed bucket */
	        index++;
	        continue;
	      }

	      /* else */
	      L->buckets[L->curr_m-1].array[L->buckets[L->curr_m-1].arr_entries] = L->buckets[L->p].array[index];
	      L->buckets[L->curr_m-1].arr_entries++;
	      memmove(L->buckets[L->p].array+index,L->buckets[L->p].array+index+1,(L->buckets[L->p].arr_entries-index-1)*sizeof(TrieNode));
	      L->buckets[L->p].arr_entries--;      
		}

	    /* Increase value of p because a split was triggered */
	    (L->p)++;
	    int u = mypow(2,L->round)*(L->m);
	    if(L->p == u){
	    	L->p = 0;
	      	(L->round)++;
		}
	}

	return 1;
}

TrieNode* LinearHashLookup(LinearHash L, char* key, int* bucket_found, int* pos_found)
{
	int first, middle, last, comp;

	int hash = HashesAt(L, key, L->round);
	if(hash < L->p)
		hash = HashesAt(L, key, L->round+1);

	Bucket bucket = L->buckets[hash];

	first = 0;
	last =  bucket.arr_entries-1;
	middle = (first+last)/2;
	
	while(first <= last){ /*Binary search*/
		comp = strcmp(key, bucket.array[middle].word);
		if(comp > 0)
			first = middle + 1;
		else if(!comp){
			break;
		}
		else 
			last = middle-1;
		
		middle = (first+last)/2;
	}

	if(first > last){
		*bucket_found = -1;
		return NULL; /*Child not found*/
	}

	*bucket_found = hash;
	*pos_found = middle;

	return &(bucket.array[middle]);
}

Bool LinearHashDeleteAt(LinearHash L, int bucket_found, int pos_found, int delete)
{
	Bucket* bucket = &(L->buckets[bucket_found]);
	if(delete){
		free(bucket->array[pos_found].word);
		free(bucket->array[pos_found].children);
	}

	if(pos_found == bucket->arr_entries-1){
		bucket->arr_entries--;
		return 1;
	}

	/* else need to shift */
	memmove(bucket->array+pos_found, bucket->array+pos_found+1, (bucket->arr_entries-pos_found-1)*sizeof(TrieNode));
	bucket->arr_entries--;
	return 1;
}

void LinearHashDestroy(LinearHash* L)
{
	int i;
	for(i = 0; i < (*L)->curr_m; i++)
		free((*L)->buckets[i].array);

	free((*L)->buckets);
	free(*L);

	*L = NULL;
}
