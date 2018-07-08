#include <stdlib.h>
#include <string.h>

#include "StaticTrie.h"
#include "StaticLinearHash.h"
#include "AssistingFunctions.h"

#include "MurmurHash2.h"

#define MURMUR_SEED 941762

int HashesAt2(StaticLinearHash L, char* key, int i)
{
	return MurmurHash2( key, strlen(key), MURMUR_SEED) % ( mypow(2,i)*(L->m) );
}

void StaticLinearHashCreate(StaticLinearHash* L, unsigned int m, unsigned int c)
{
	int i;

	(*L) = malloc(sizeof(StaticLinearHash_t));
	(*L)->m = m;
	(*L)->curr_m = m;
	(*L)->c = c;
	(*L)->p = 0;
	(*L)->round = 0;
	(*L)->buckets = malloc(m*sizeof(StaticBucket));

	for(i = 0; i < m; i++)
	{
		(*L)->buckets[i].array = malloc(c*sizeof(HyperNode));
		(*L)->buckets[i].arr_size = c;
		(*L)->buckets[i].arr_entries = 0;
	}
}

Bool StaticLinearHashInsert(StaticLinearHash L, HyperNode n)
{
	/* split remembers if there was an overflow,
	and if there was, perform the split */
	int first, middle, last, comp,i, split = 0;

	int hash = HashesAt2(L, n.word, L->round);
	if(hash < L->p)
		hash = HashesAt2(L, n.word, L->round+1);

	StaticBucket* bucket = &(L->buckets[hash]);

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
		bucket->array = realloc(bucket->array, 2*bucket->arr_size*sizeof(HyperNode));
		bucket->arr_size *= 2;
		split = 1;
	}
 			
    memmove(bucket->array+first+1, bucket->array+first,(bucket->arr_entries-first)*sizeof(HyperNode));
    bucket->arr_entries++;
    bucket->array[first] = n;
	  
	if(split){  
	    /* Perform the split */
	    L->curr_m++;
	    
	    L->buckets = realloc(L->buckets,(L->curr_m)*sizeof(StaticBucket));
		
		/* The new bucket will have the same size as the one to be split.
		That is so that there will be no chain reaction of splits. */
	    L->buckets[L->curr_m-1].array = malloc(L->buckets[L->p].arr_size*sizeof(HyperNode));
	    L->buckets[L->curr_m-1].arr_size = L->buckets[L->p].arr_size;
	    L->buckets[L->curr_m-1].arr_entries = 0;

		int hashes_at;
	    int index = 0; /* This index points to the next element that we will check if it needs redistribution. */
		while(index < L->buckets[L->p].arr_entries){
	      hashes_at = HashesAt2(L, L->buckets[L->p].array[index].word, L->round+1);
	      if(hashes_at == L->p){ /* If hashes on the same, leave it there, else move to the newly formed bucket */
	        index++;
	        continue;
	      }

	      /* else */
	      L->buckets[L->curr_m-1].array[L->buckets[L->curr_m-1].arr_entries] = L->buckets[L->p].array[index];
	      L->buckets[L->curr_m-1].arr_entries++;
	      memmove(L->buckets[L->p].array+index,L->buckets[L->p].array+index+1,(L->buckets[L->p].arr_entries-index-1)*sizeof(HyperNode));
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

HyperNode* StaticLinearHashLookup(StaticLinearHash L, char* key, int* bucket_found, int* pos_found)
{
	int first, middle, last, comp;

	int hash = HashesAt2(L, key, L->round);
	if(hash < L->p)
		hash = HashesAt2(L, key, L->round+1);

	StaticBucket bucket = L->buckets[hash];

	first = 0;
	last =  bucket.arr_entries-1;
	middle = (first+last)/2;
	
	while(first <= last){ /*Binary search*/
		
		int count;
		int absol = abs(bucket.array[middle].lengths[0]);

		/*if(strlen(key) < absol)
			count = absol;
		else count = strlen(key);

		comp = strncmp(key,bucket.array[middle].word,count);*/

		comp = strncmp(key,bucket.array[middle].word,absol);
		if(absol != strlen(key)){
			if(comp == 0){
				if(absol < strlen(key)) comp = 1;
				else comp = -1;
			}	
		}
		
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

void StaticLinearHashDestroy(StaticLinearHash* L)
{
	int i;
	for(i = 0; i < (*L)->curr_m; i++)
		free((*L)->buckets[i].array);

	free((*L)->buckets);
	free(*L);

	*L = NULL;
}
