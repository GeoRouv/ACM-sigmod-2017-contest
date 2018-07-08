#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "./../src/Queue.h"
#include "./Unity-master/src/unity.h"

typedef char Bool;
typedef struct Trie_t* Trie;

Bool deleted;

/* Every Bool function returns 0 on success, 1 on failure */

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
	int len = strlen(s); /* Compiler might optimize this? */

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
	if(found && next_token > 0){
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






/* Test functions */

void test_TrieAdd_EmptyTrie()
{
	/*Check if the insertion of a single n-gram to an empty trie is done the right way*/
	Trie trie;
	TrieCreate(&trie, 10);

	char buf[100];
	strcpy(buf, "Hello how are you");

	TrieAdd(trie, buf);
	TEST_ASSERT_NULL(trie->root->word);
	TEST_ASSERT_NOT_NULL(trie->root->children);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, trie->root->numChildren, "First insert->Root not 1 child");
	/*Check the arraySize of each node-It should be 10 for all of them*/
	TEST_ASSERT_EQUAL_INT(10,trie->root->arraySize);
	TEST_ASSERT_EQUAL_INT(10, trie->root->children[0].arraySize);
	TEST_ASSERT_EQUAL_INT(10, trie->root->children[0].children[0].arraySize);
	TEST_ASSERT_EQUAL_INT(10, trie->root->children[0].children[0].children[0].arraySize);
	TEST_ASSERT_EQUAL_INT(10, trie->root->children[0].children[0].children[0].children[0].arraySize);
	/*Check if the words of the n-gram are where they should be*/
	TEST_ASSERT_EQUAL_STRING("Hello", trie->root->children[0].word);
	TEST_ASSERT_EQUAL_STRING("how", trie->root->children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("are", trie->root->children[0].children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("you", trie->root->children[0].children[0].children[0].children[0].word);
	/*Check the "isFinal" status in each word in the n-gram*/
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].children[0].children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[0].children[0].children[0].children[0].isFinal);

	TrieDestroy(&trie);
	TEST_ASSERT_NULL(trie);
	
}

void test_TrieAdd_NonEmptyTrie()
{
	/*Check if the insertion to a non empty trie is done the right way*/
	Trie trie;
	TrieCreate(&trie, 10);

	char buf[100];
	
	strcpy(buf, "Hello how are you");
	TrieAdd(trie, buf);

	strcpy(buf, "This is a camel");
	TrieAdd(trie, buf);

	strcpy(buf, "Accelerating atoms through space");
	TrieAdd(trie, buf);

	TEST_ASSERT_EQUAL_INT(10,trie->root->arraySize);
	TEST_ASSERT_EQUAL_INT_MESSAGE(3, trie->root->numChildren, "First insert->Root not 3 children");
	/*Check if every word is in their corresponding position taking into account the alphabetical order*/
	TEST_ASSERT_EQUAL_STRING("Accelerating", trie->root->children[0].word);
	TEST_ASSERT_EQUAL_STRING("atoms", trie->root->children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("through", trie->root->children[0].children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("space", trie->root->children[0].children[0].children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("Hello", trie->root->children[1].word);
	TEST_ASSERT_EQUAL_STRING("how", trie->root->children[1].children[0].word);
	TEST_ASSERT_EQUAL_STRING("are", trie->root->children[1].children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("you", trie->root->children[1].children[0].children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("This", trie->root->children[2].word);
	TEST_ASSERT_EQUAL_STRING("is", trie->root->children[2].children[0].word);
	TEST_ASSERT_EQUAL_STRING("a", trie->root->children[2].children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("camel", trie->root->children[2].children[0].children[0].children[0].word);
	/*Check the final words in each n-gram*/
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[0].children[0].children[0].children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[1].children[0].children[0].children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[2].children[0].children[0].children[0].isFinal);

	TrieDestroy(&trie);
	TEST_ASSERT_NULL(trie);

}

void test_TrieAdd_AlreadyExists()
{
	/*Check the behavior when we insert an existing n-gram to the trie*/
	Trie trie;
	TrieCreate(&trie, 10);

	char buf[100];
	
	strcpy(buf, "Hello how are you");
	TrieAdd(trie, buf);

	strcpy(buf, "Hello how are you");
	TrieAdd(trie, buf);

	/*Nothing should change-Its as if we have inserted 1 n-gram*/
	TEST_ASSERT_EQUAL_INT(10, trie->root->arraySize);
	/*Every node except the last should have 1 children*/
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, trie->root->numChildren, "First insert->Root not 1 child");
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[0].numChildren);
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[0].children[0].numChildren);
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[0].children[0].children[0].numChildren);
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].children[0].children[0].children[0].numChildren);
	/*Check if the words of the n-gram are where they should be*/
	TEST_ASSERT_EQUAL_STRING("Hello", trie->root->children[0].word);
	TEST_ASSERT_EQUAL_STRING("how", trie->root->children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("are", trie->root->children[0].children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("you", trie->root->children[0].children[0].children[0].children[0].word);
	/*Check the "isFinal" status of each word in the n-gram*/
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].children[0].children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[0].children[0].children[0].children[0].isFinal);

	TrieDestroy(&trie);
	TEST_ASSERT_NULL(trie);

}

void test_TrieAdd_PartiallyExists()
{
	/*Check the behavior when we insert an n-gram which causes an inner node to create another child*/
	Trie trie;
	TrieCreate(&trie, 10);

	char buf[100];
	
	strcpy(buf, "Hello how are you");
	TrieAdd(trie, buf);

	strcpy(buf, "Hello how was your day");
	TrieAdd(trie, buf);	

	/*Every node except the one with "how" word should have 1 children*/
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, trie->root->numChildren, "First insert->Root not 1 child");
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[0].numChildren);
	TEST_ASSERT_EQUAL_INT(2, trie->root->children[0].children[0].numChildren);
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[0].children[0].children[0].numChildren);
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].children[0].children[0].children[0].numChildren);
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[0].children[0].children[1].numChildren);
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[0].children[0].children[1].children[0].numChildren);
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].children[0].children[1].children[0].children[0].numChildren);
	/*Check the "isFinal" status of each word*/
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].children[0].children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[0].children[0].children[0].children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].children[0].children[1].isFinal);
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].children[0].children[1].children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[0].children[0].children[1].children[0].children[0].isFinal);	
	/*Check if every word is where it should be*/
	TEST_ASSERT_EQUAL_STRING("Hello", trie->root->children[0].word);
	TEST_ASSERT_EQUAL_STRING("how", trie->root->children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("are", trie->root->children[0].children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("you", trie->root->children[0].children[0].children[0].children[0].word);	
	TEST_ASSERT_EQUAL_STRING("was", trie->root->children[0].children[0].children[1].word);
	TEST_ASSERT_EQUAL_STRING("your", trie->root->children[0].children[0].children[1].children[0].word);
	TEST_ASSERT_EQUAL_STRING("day", trie->root->children[0].children[0].children[1].children[0].children[0].word);

	TrieDestroy(&trie);
	TEST_ASSERT_NULL(trie);

}

void test_TrieAdd_AllWordsAlreadyInTrie()
{
	/*Check the behavior when we insert an n-gram which is the beggining of an existing n-gram in the trie*/
	Trie trie;
	TrieCreate(&trie, 10);

	char buf[100];
	
	strcpy(buf, "Hello how are you");
	TrieAdd(trie, buf);

	strcpy(buf, "Hello how are");
	TrieAdd(trie, buf);

	/*Every node has the same number of children as before*/
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, trie->root->numChildren, "First insert->Root not 1 child");
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[0].numChildren);
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[0].children[0].numChildren);
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[0].children[0].children[0].numChildren);
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].children[0].children[0].children[0].numChildren);
	/*Check if the words are in the right place*/
	TEST_ASSERT_EQUAL_STRING("Hello", trie->root->children[0].word);
	TEST_ASSERT_EQUAL_STRING("how", trie->root->children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("are", trie->root->children[0].children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("you", trie->root->children[0].children[0].children[0].children[0].word);
	/*Check if the "isFinal" status of the "are" word was changed*/
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[0].children[0].children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[0].children[0].children[0].children[0].isFinal);

	TrieDestroy(&trie);
	TEST_ASSERT_NULL(trie);

}

void test_TrieAdd_InvokeArrayRealloc()
{
	/*Check the behavior when we insert more nodes to a level than the arraySize*/
	Trie trie;
	TrieCreate(&trie, 2);

	char buf[100];
	
	strcpy(buf, "Hello how are you");
	TrieAdd(trie, buf);

	TEST_ASSERT_EQUAL_INT(2,trie->root->arraySize);
	TEST_ASSERT_EQUAL_INT(1, trie->root->numChildren);

	strcpy(buf, "This is a camel");
	TrieAdd(trie, buf);
	/*Array size should still be 2*/
	TEST_ASSERT_EQUAL_INT(2,trie->root->arraySize);

	strcpy(buf, "Accelerating atoms through space");
	TrieAdd(trie, buf);
	/*Array size for the root node has changed*/
	TEST_ASSERT_EQUAL_INT(4,trie->root->arraySize);

	/*Other arraySize's dont need to change*/
	TEST_ASSERT_EQUAL_INT(2, trie->root->children[0].arraySize);
	TEST_ASSERT_EQUAL_INT(2, trie->root->children[0].children[0].arraySize);
	TEST_ASSERT_EQUAL_INT(2, trie->root->children[0].children[0].children[0].arraySize);
	TEST_ASSERT_EQUAL_INT(2, trie->root->children[0].children[0].children[0].children[0].arraySize);
	/*Check if the n-gram that invoked the array doubling is inserted successfully*/
	TEST_ASSERT_EQUAL_STRING("Accelerating", trie->root->children[0].word);
	TEST_ASSERT_EQUAL_STRING("atoms", trie->root->children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("through", trie->root->children[0].children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("space", trie->root->children[0].children[0].children[0].children[0].word);
	/*Check if the other n-grams are in the right position*/
	TEST_ASSERT_EQUAL_STRING("Hello", trie->root->children[1].word);
	TEST_ASSERT_EQUAL_STRING("how", trie->root->children[1].children[0].word);
	TEST_ASSERT_EQUAL_STRING("are", trie->root->children[1].children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("you", trie->root->children[1].children[0].children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("This", trie->root->children[2].word);
	TEST_ASSERT_EQUAL_STRING("is", trie->root->children[2].children[0].word);
	TEST_ASSERT_EQUAL_STRING("a", trie->root->children[2].children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("camel", trie->root->children[2].children[0].children[0].children[0].word);

	TrieDestroy(&trie);
	TEST_ASSERT_NULL(trie);

}

void test_TrieAdd_WhiteSpace()
{
	/*Check if the Add function overlooks any unwanted white space*/
	Trie trie;
	TrieCreate(&trie, 10);

	char buf[100];

	/*Add should return 1*/
	strcpy(buf, " \t");
	TEST_ASSERT_EQUAL_INT(1,TrieAdd(trie, buf));

	/*Check if nothing has been added to the trie*/
	TEST_ASSERT_NULL(trie->root->word);
	TEST_ASSERT_EQUAL_INT(0, trie->root->numChildren);
	TEST_ASSERT_EQUAL_INT(10,trie->root->arraySize);

	/*Check that unwanted white space has been eliminated*/
	strcpy(buf, "    \tHello how \t are     you       ");
	TrieAdd(trie, buf);

	/*Check that every word is in their corresponding position with no white space*/
	TEST_ASSERT_EQUAL_STRING("Hello", trie->root->children[0].word);
	TEST_ASSERT_EQUAL_STRING("how", trie->root->children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("are", trie->root->children[0].children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("you", trie->root->children[0].children[0].children[0].children[0].word);
	/*Check the "isFinal" status in each word in the n-gram*/
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].children[0].children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[0].children[0].children[0].children[0].isFinal);

	TrieDestroy(&trie);
	TEST_ASSERT_NULL(trie);

}

void test_TrieQuestion_InTrie()
{
	/*Check if question for an already existing n-gram works as expected*/
	Trie trie;
	TrieCreate(&trie, 10);

	char buf[100];
	char q_result[100];
	q_result[0] = '\0';

	strcpy(buf, "Hello how are you");
	TrieAdd(trie, buf);

	/*Question with the exact trie n-gram*/
	strcpy(buf, "Hello how are you");
	Queue q = TrieQuestion(trie, buf);

	char* tmp;
	while(tmp = (char*) QueuePop(q)){
		sprintf(q_result+strlen(q_result),"%s|",tmp);
		free(tmp);
	}
	q_result[strlen(q_result)-1] = '\0';

	TEST_ASSERT_EQUAL_STRING("Hello how are you", q_result);

	QueueDestroy(&q);

	q_result[0] = '\0';

	/*Question which contains the trie n-gram*/
	strcpy(buf, "Hello how are you my friend");
	q = TrieQuestion(trie, buf);

	while(tmp = (char*) QueuePop(q)){
		sprintf(q_result+strlen(q_result),"%s|",tmp);
		free(tmp);
	}
	q_result[strlen(q_result)-1] = '\0';

	TEST_ASSERT_EQUAL_STRING("Hello how are you", q_result);
	
	QueueDestroy(&q);
	TrieDestroy(&trie);
	TEST_ASSERT_NULL(trie);

}

void test_TrieQuestion_PartiallyInTrie()
{
	/*Check the behavior if question contains an n-gram which partially exists in the trie*/
	Trie trie;
	TrieCreate(&trie, 10);

	char buf[100];
	char q_result[100];
	q_result[0] = '\0';

	strcpy(buf, "Hello how are you");
	TrieAdd(trie, buf);

	strcpy(buf, "Hello how are");
	Queue q = TrieQuestion(trie, buf);

	TEST_ASSERT_NULL(q);

	TrieDestroy(&trie);
	TEST_ASSERT_NULL(trie);

}

void test_TrieQuestion_NotInTrie()
{
	/*Check the question result if the given n gram is not in the trie*/
	Trie trie;
	TrieCreate(&trie, 10);

	char buf[100];
	char q_result[100];
	q_result[0] = '\0';

	strcpy(buf, "Hello how are you");
	TrieAdd(trie, buf);

	strcpy(buf, "Space station in between");
	Queue q = TrieQuestion(trie, buf);

	TEST_ASSERT_NULL(q);

	TrieDestroy(&trie);
	TEST_ASSERT_NULL(trie);

}

void test_TrieQuestion_MultipleNgrams()
{
	/*Check the question result if there are multiple n-grams in the question */
	Trie trie;
	TrieCreate(&trie, 10);
	
	char buf[100];
	char q_result[100];
	q_result[0] = '\0';

	strcpy(buf, "This is");
	TrieAdd(trie, buf);

	strcpy(buf, "This is a dog");
	TrieAdd(trie, buf);

	strcpy(buf, "cat");
	TrieAdd(trie, buf);

	strcpy(buf, "is a cat");
	TrieAdd(trie, buf);

	strcpy(buf, "This is a dog and this is a cat");
	Queue q = TrieQuestion(trie, buf);

	char* tmp;
	while(tmp = (char*) QueuePop(q)){
		sprintf(q_result+strlen(q_result),"%s|",tmp);
		free(tmp);
	}
	q_result[strlen(q_result)-1] = '\0';

	TEST_ASSERT_EQUAL_STRING("This is|This is a dog|is a cat|cat", q_result);

	QueueDestroy(&q);
	TrieDestroy(&trie);
	TEST_ASSERT_NULL(trie);

}

void test_TrieQuestion_WhiteSpace()
{
	/*Check if the Question function overlooks any unwanted white space*/
	Trie trie;
	TrieCreate(&trie, 10);

	char buf[100];
	char q_result[100];
	q_result[0] = '\0';

	strcpy(buf, "Hello how are you");
	TrieAdd(trie, buf);	

	/*Question should return NULL*/
	strcpy(buf, " \t");
	TEST_ASSERT_NULL(TrieQuestion(trie, buf));

	/*Check that unwanted white space has been eliminated from the question*/
	strcpy(buf, "    \tHello how \t are     you       ");
	Queue q = TrieQuestion(trie, buf);

	char* tmp;
	while(tmp = (char*) QueuePop(q)){
		sprintf(q_result+strlen(q_result),"%s|",tmp);
		free(tmp);
	}
	q_result[strlen(q_result)-1] = '\0';

	TEST_ASSERT_EQUAL_STRING("Hello how are you", q_result);

	QueueDestroy(&q);
	TrieDestroy(&trie);
	TEST_ASSERT_NULL(trie);

}

void test_TrieDelete_ExistsInTrieNoOtherFinalInPath()
{
	/*Check the behavior if we delete an n-gram which has only 1 final in its path*/
	Trie trie;
	TrieCreate(&trie, 10);

	char buf[100];
	char q_result[100];
	q_result[0] = '\0';

	strcpy(buf, "Hello how are you");
	TrieAdd(trie, buf);

	strcpy(buf, "This is a camel");
	TrieAdd(trie, buf);

	strcpy(buf, "Hello how are you");
	Queue q = TrieQuestion(trie, buf);

	char* tmp;
	while(tmp = (char*) QueuePop(q)){
		sprintf(q_result+strlen(q_result),"%s|",tmp);
		free(tmp);
	}
	q_result[strlen(q_result)-1] = '\0';

	TEST_ASSERT_EQUAL_STRING("Hello how are you", q_result);

	q_result[0] = '\0';

	strcpy(buf, "Hello how are you");
	TEST_ASSERT_EQUAL_INT(0, TrieDelete(trie,buf));
	
	strcpy(buf, "Hello how are you");
	Queue q1 = TrieQuestion(trie, buf);

	/*N-gram was deleted so if you make a quesion about it, -1 should be returned*/
	TEST_ASSERT_NULL(q1);
	
	/*Check the number of children of the remaining nodes*/
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, trie->root->numChildren, "First insert->Root not 1 child");
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[0].numChildren);
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[0].children[0].numChildren);
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[0].children[0].children[0].numChildren);
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].children[0].children[0].children[0].numChildren);
	/*Check if the words of the remaining n-gram have shifted left*/
	TEST_ASSERT_EQUAL_STRING("This", trie->root->children[0].word);
	TEST_ASSERT_EQUAL_STRING("is", trie->root->children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("a", trie->root->children[0].children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("camel", trie->root->children[0].children[0].children[0].children[0].word);	

	QueueDestroy(&q);
	TrieDestroy(&trie);
	TEST_ASSERT_NULL(trie);

}

void test_TrieDelete_ExistsInTrieOtherFinalInPath()
{
	/*Check the behavior if we delete an n-gram which contains another n-gram in its path*/
	Trie trie;
	TrieCreate(&trie, 10);

	char buf[100];
	char q_result[100];
	q_result[0] = '\0';

	strcpy(buf, "Hello how are you");
	TrieAdd(trie, buf);

	strcpy(buf, "Hello how are");
	TrieAdd(trie, buf);

	strcpy(buf, "Hello how are you");
	TEST_ASSERT_EQUAL_INT(0, TrieDelete(trie,buf));

	/*Check if "Hello how are" is still in the trie*/
	strcpy(buf, "Hello how are");
	Queue q = TrieQuestion(trie, buf);

	char* tmp;
	while(tmp = (char*) QueuePop(q)){
		sprintf(q_result+strlen(q_result),"%s|",tmp);
		free(tmp);
	}
	q_result[strlen(q_result)-1] = '\0';

	TEST_ASSERT_EQUAL_STRING("Hello how are", q_result);

	QueueDestroy(&q);

	q_result[0] = '\0';

	/*Check that the "Hello how are you" n-gram cannot be found in the trie*/
	strcpy(buf, "Hello how are you");
	q = TrieQuestion(trie, buf);

	while(tmp = (char*) QueuePop(q)){
		sprintf(q_result+strlen(q_result),"%s|",tmp);
		free(tmp);
	}
	q_result[strlen(q_result)-1] = '\0';

	TEST_ASSERT_EQUAL_STRING("Hello how are", q_result);

	QueueDestroy(&q);
	TrieDestroy(&trie);
	TEST_ASSERT_NULL(trie);

}

void test_TrieDelete_NotInTrieAtAll()
{
	/*Check the behavior if we delete an n-gram that does not exist in the trie-Delete should return 1*/
	Trie trie;
	TrieCreate(&trie, 10);
	char buf[100];
	
	strcpy(buf, "Hello how are you");
	TrieAdd(trie, buf);

	strcpy(buf, "This is a camel");
	TEST_ASSERT_EQUAL_INT(1,TrieDelete(trie,buf));

	TrieDestroy(&trie);
	TEST_ASSERT_NULL(trie);

}

void test_TrieDelete_InTriePartiallyStart()
{
	/*Check the behavior if we delete an n-gram that doesnt exist in the trie but has a common start with an existing one*/
	Trie trie;
	TrieCreate(&trie, 10);
	char buf[100];
	
	strcpy(buf, "Hello how are you");
	TrieAdd(trie, buf);

	strcpy(buf, "Hello how are");
	TEST_ASSERT_EQUAL_INT(1,TrieDelete(trie,buf));

	/*Check that nothing has changed*/
	TEST_ASSERT_EQUAL_STRING("Hello", trie->root->children[0].word);
	TEST_ASSERT_EQUAL_STRING("how", trie->root->children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("are", trie->root->children[0].children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("you", trie->root->children[0].children[0].children[0].children[0].word);
	/*Check the "isFinal" status in each word in the n-gram*/
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].children[0].children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[0].children[0].children[0].children[0].isFinal);	

	TrieDestroy(&trie);
	TEST_ASSERT_NULL(trie);

}

void test_TrieDelete_InTriePartiallyEnd()
{
	/*Check the behavior if we delete an n-gram that doesnt exist in the trie but has a common end with an existing one*/
	Trie trie;
	TrieCreate(&trie, 10);
	char buf[100];
	
	strcpy(buf, "Hello how are you");
	TrieAdd(trie, buf);

	strcpy(buf, "how are you");
	TEST_ASSERT_EQUAL_INT(1,TrieDelete(trie,buf));

	/*Check that nothing has changed*/
	TEST_ASSERT_EQUAL_STRING("Hello", trie->root->children[0].word);
	TEST_ASSERT_EQUAL_STRING("how", trie->root->children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("are", trie->root->children[0].children[0].children[0].word);
	TEST_ASSERT_EQUAL_STRING("you", trie->root->children[0].children[0].children[0].children[0].word);
	/*Check the "isFinal" status in each word in the n-gram*/
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(0, trie->root->children[0].children[0].children[0].isFinal);
	TEST_ASSERT_EQUAL_INT(1, trie->root->children[0].children[0].children[0].children[0].isFinal);	

	TrieDestroy(&trie);
	TEST_ASSERT_NULL(trie);

}

int main(void)
{
	UNITY_BEGIN();

	/* RUN TESTS */
	RUN_TEST(test_TrieAdd_EmptyTrie);
	RUN_TEST(test_TrieAdd_NonEmptyTrie);
	RUN_TEST(test_TrieAdd_AlreadyExists);
	RUN_TEST(test_TrieAdd_PartiallyExists);
	RUN_TEST(test_TrieAdd_AllWordsAlreadyInTrie);
	RUN_TEST(test_TrieAdd_InvokeArrayRealloc);
	RUN_TEST(test_TrieAdd_WhiteSpace);

	RUN_TEST(test_TrieQuestion_InTrie);
	RUN_TEST(test_TrieQuestion_PartiallyInTrie);
	RUN_TEST(test_TrieQuestion_NotInTrie);
	RUN_TEST(test_TrieQuestion_MultipleNgrams);
	RUN_TEST(test_TrieQuestion_WhiteSpace);

	RUN_TEST(test_TrieDelete_ExistsInTrieNoOtherFinalInPath);
	RUN_TEST(test_TrieDelete_ExistsInTrieOtherFinalInPath);
	RUN_TEST(test_TrieDelete_NotInTrieAtAll);
	RUN_TEST(test_TrieDelete_InTriePartiallyStart);
	RUN_TEST(test_TrieDelete_InTriePartiallyEnd);

	return UNITY_END();

}