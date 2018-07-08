#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "AssistingFunctions.h"


Bool isFlag(char* word)
{
	if(word[0] == '-' && strlen(word) == 2)
		return 1;
	return 0;
}

void print_usage(char* prog_name)
{
	printf("%s usage: ./ngrams -i <init_file> -q <query_file>\n", prog_name);
}

void MapInsert(char* ngram,ngram_freq** array,int* size,int* entries)
{
	int first,last,middle,comp,found;
	found = 0;

	first = 0;
	last =  (*entries)-1;
	middle = (first+last)/2;

	while(first <= last){ /*Binary search*/
		comp = strcmp(ngram,(*array)[middle].ngram);
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

	if(first > last){ /*Child not found*/	
		found = 0;
	}
	
	if(found){ /*Increment the times of appearance*/
		(*array)[middle].times++;
		free(ngram);
	}
	else{
		if(*entries == *size){
			(*array) = realloc((*array), 2*(*size)*sizeof(ngram_freq));
			*size = 2*(*size);
		} 
		memmove((*array)+first+1, (*array)+first, (*entries-first)*sizeof(ngram_freq));
		(*array)[first].ngram = ngram;
		(*array)[first].times = 1;
		(*entries)++;
	}
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

int mypow(int x, int y)
{
	/* Calculate x^y */
	int i;
	int result = 1;
	for(i = 0; i < y; i++)
		result *= x;

	return result;
}
