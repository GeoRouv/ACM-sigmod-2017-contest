#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../src/Queue.h"
#include "../src/DynamicTrie.h"
#include "../src/BloomFilter.h"
#include "../src/LinearHash.h"
#include "../src/AssistingFunctions.h"
#include "../src/StaticLinearHash.h"
#include "../src/StaticTrie.h"
#include "./Unity-master/src/unity.h"

void test_DynamicTrieAdd_oneNgram()
{
	Trie trie;
	DynamicTrieCreate((void*)&trie, 10);

	char buffer[100];
	strcpy(buffer, "Hello how are you");

	DynamicTrieAdd(trie, buffer);

	int x,y;
	TrieNode* t = LinearHashLookup((LinearHash)(trie->root->children), "Hello", &x, &y);

	TEST_ASSERT_EQUAL_STRING(t->word, "Hello");
	TEST_ASSERT_EQUAL_STRING(t->children[0].word, "how");
	TEST_ASSERT_EQUAL_STRING(t->children[0].children[0].word, "are");
	TEST_ASSERT_EQUAL_STRING(t->children[0].children[0].children[0].word, "you");

	DynamicTrieDestroy((void*)&trie);	 
}

void test_DynamicTrieAdd_multipleNgrams()
{
	Trie trie;
	DynamicTrieCreate((void*)&trie, 10);

	char buffer[100];
	strcpy(buffer, "Hello how are you");
	DynamicTrieAdd(trie, buffer);
	strcpy(buffer, "Hello how are they");
	DynamicTrieAdd(trie, buffer);
	strcpy(buffer, "My name is Jeff");
	DynamicTrieAdd(trie, buffer);

	int x,y;
	TrieNode* t = LinearHashLookup((LinearHash)(trie->root->children), "Hello", &x, &y);

	TEST_ASSERT_EQUAL_STRING(t->word, "Hello");
	TEST_ASSERT_EQUAL_STRING(t->children[0].word, "how");
	TEST_ASSERT_EQUAL_STRING(t->children[0].children[0].word, "are");
	TEST_ASSERT_EQUAL_STRING(t->children[0].children[0].children[0].word, "they");
	TEST_ASSERT_EQUAL_STRING(t->children[0].children[0].children[1].word, "you");

	t = LinearHashLookup((LinearHash)(trie->root->children), "My", &x, &y);

	TEST_ASSERT_EQUAL_STRING(t->word, "My");
	TEST_ASSERT_EQUAL_STRING(t->children[0].word, "name");
	TEST_ASSERT_EQUAL_STRING(t->children[0].children[0].word, "is");
	TEST_ASSERT_EQUAL_STRING(t->children[0].children[0].children[0].word, "Jeff");

	DynamicTrieDestroy((void*)&trie);
}

void test_DynamicTrieAdd_AlreadyExists()
{
	Trie trie;
	DynamicTrieCreate((void*)&trie, 10);

	char buffer[100];
	strcpy(buffer, "Hello how are you");
	DynamicTrieAdd(trie, buffer);
	strcpy(buffer, "Hello how are you");
	DynamicTrieAdd(trie, buffer);

	int x,y;
	TrieNode* t = LinearHashLookup((LinearHash)(trie->root->children), "Hello", &x, &y);

	TEST_ASSERT_EQUAL_INT(1, t->numChildren);
	TEST_ASSERT_EQUAL_INT(1, t->children[0].numChildren);
	TEST_ASSERT_EQUAL_INT(1, t->children[0].children[0].numChildren);	
	TEST_ASSERT_EQUAL_INT(0, t->children[0].children[0].children[0].numChildren);

	DynamicTrieDestroy((void*)&trie);
}

void test_TrieAdd_InvokeArrayRealloc()
{
	Trie trie;
	DynamicTrieCreate((void*)&trie, 3);

	char buffer[100];
	strcpy(buffer, "Hello how are you");
	DynamicTrieAdd(trie, buffer);
	strcpy(buffer, "Hello mister");
	DynamicTrieAdd(trie, buffer);
	strcpy(buffer, "Hello man what's up");
	DynamicTrieAdd(trie, buffer);
	strcpy(buffer, "Hello to my fans");
	DynamicTrieAdd(trie, buffer);

	int x,y;
	TrieNode* t = LinearHashLookup((LinearHash)(trie->root->children), "Hello", &x, &y);

	TEST_ASSERT_EQUAL_INT(4, t->numChildren);
	TEST_ASSERT_EQUAL_STRING(t->word, "Hello");
	TEST_ASSERT_EQUAL_STRING(t->children[0].word, "how");
	TEST_ASSERT_EQUAL_STRING(t->children[1].word, "man");
	TEST_ASSERT_EQUAL_STRING(t->children[2].word, "mister");
	TEST_ASSERT_EQUAL_STRING(t->children[3].word, "to");

	DynamicTrieDestroy((void*)&trie);
}

void test_DynamicTrieAdd_WhiteSpace()
{
	Trie trie;
	DynamicTrieCreate((void*)&trie, 3);

	char buffer[100];
	strcpy(buffer, "  \t\t \t  \t\t     \t");

	TEST_ASSERT_EQUAL_INT(1, DynamicTrieAdd((void*)trie, buffer));
	
	strcpy(buffer, " what   \t is \t\t   love\t\t\t \t\t ");
	DynamicTrieAdd((void*)trie, buffer);

	int x,y;
	TrieNode* t = LinearHashLookup((LinearHash)(trie->root->children), "what", &x, &y);

	TEST_ASSERT_EQUAL_STRING(t->word, "what");
	TEST_ASSERT_EQUAL_STRING(t->children[0].word, "is");
	TEST_ASSERT_EQUAL_STRING(t->children[0].children[0].word, "love");

	DynamicTrieDestroy((void*)&trie);	
}

void test_DynamicTrie_QuestionAndDelete()
{
	Trie trie;
	DynamicTrieCreate((void*)&trie, 3);
	char* s;

	char buffer[100];
	strcpy(buffer, "My name is Kartoffel");
	DynamicTrieAdd((void*)trie, buffer);
	strcpy(buffer, "My name is Slim Shady");
	DynamicTrieAdd((void*)trie, buffer);
	strcpy(buffer, "My name is");
	DynamicTrieAdd((void*)trie, buffer);
	strcpy(buffer, "Tsoukou tsoukou slim shady");
	DynamicTrieAdd((void*)trie, buffer);
	strcpy(buffer, "Tsou tsou thomas to treno perna");
	DynamicTrieAdd((void*)trie, buffer);

	Queue q;

	strcpy(buffer, "My name is Tsou tsou thomas to treno perna");
	q = DynamicTrieQuestion(trie, buffer);
	TEST_ASSERT_EQUAL_STRING("My name is", s = (char*)QueuePop(q));
	free(s);
	TEST_ASSERT_EQUAL_STRING("Tsou tsou thomas to treno perna", s = (char*)QueuePop(q));
	free(s);
	QueueDestroy(&q);

	strcpy(buffer, "My name is");
	TEST_ASSERT_EQUAL_INT(0, DynamicTrieDelete(trie, buffer));

	strcpy(buffer, "My name is Tsou tsou thomas to treno perna");
	q = DynamicTrieQuestion(trie, buffer);
	TEST_ASSERT_EQUAL_STRING("Tsou tsou thomas to treno perna", s = (char*)QueuePop(q));
	free(s);

	DynamicTrieDestroy((void*)&trie);
	QueueDestroy(&q);
}

void test_DynamicTrie_DeleteFails()
{
	Trie trie;
	DynamicTrieCreate((void*)&trie, 3);
	char buffer[100];

	strcpy(buffer, "Hsouna ksipoliti kai mazeves radikia");
	TEST_ASSERT_EQUAL_INT(1, DynamicTrieDelete(trie, buffer));

	strcpy(buffer, "AI will prevail");
	DynamicTrieAdd(trie, buffer);

	strcpy(buffer, "AI will kill us all");
	TEST_ASSERT_EQUAL_INT(1, DynamicTrieDelete(trie, buffer));

	DynamicTrieDestroy((void*)&trie);
}

void test_BloomFilter()
{
	BloomFilter b;
	BloomFilterCreate(&b);

	TEST_ASSERT_EQUAL(1, BloomFilterAsk(b, "Good morning boi"));
	TEST_ASSERT_EQUAL(0, BloomFilterAsk(b, "Good morning boi"));

	BloomFilterReset(b);

	TEST_ASSERT_EQUAL(1, BloomFilterAsk(b, "Good morning boi"));
	TEST_ASSERT_EQUAL(0, BloomFilterAsk(b, "Good morning boi"));

	BloomFilterDestroy(&b);
}

void test_TopK()
{
	
	Trie trie;
	DynamicTrieCreate((void*)&trie, 3);
	char* s;
	int i;

	char buffer[100];
	strcpy(buffer, "My name is Kartoffel");
	DynamicTrieAdd((void*)trie, buffer);
	strcpy(buffer, "My name is Slim Shady");
	DynamicTrieAdd((void*)trie, buffer);
	strcpy(buffer, "My name is");
	DynamicTrieAdd((void*)trie, buffer);
	strcpy(buffer, "Tsoukou tsoukou slim shady");
	DynamicTrieAdd((void*)trie, buffer);
	strcpy(buffer, "Tsou tsou thomas to treno perna");
	DynamicTrieAdd((void*)trie, buffer);

	Queue res_queue;
	Queue burst;
	QueueCreate(&burst);

	strcpy(buffer,"Hello My name is Slim Shady and here is Tsoukou tsoukou slim shady");
	QueuePush(burst,(void*)DynamicTrieQuestion(trie,buffer));

	strcpy(buffer,"Hello My name is Tsiko and in there My name is Kartoffel");
	QueuePush(burst,(void*)DynamicTrieQuestion(trie,buffer));

	ngram_freq* freq_array = malloc(10*sizeof(ngram_freq));
	int arr_size = 10;
	int arr_entries = 0;

	int k = 3;

	/*Pop every result-queue from the burst queue*/
	while((res_queue = (Queue)QueuePop(burst))){
		char* s = (char*) QueuePop(res_queue);
		//printf("%s", s);
		if(!strcmp(s,"-1")){
			//printf("\n");
			free(s);
			QueueDestroy(&res_queue);
			continue;
		}
		
		MapInsert(s,&freq_array,&arr_size,&arr_entries);

		/*Pop ngrams from the result-queue*/
		while( (s=(char*)QueuePop(res_queue)) ){
			//printf("|%s", s);
			MapInsert(s,&freq_array,&arr_size,&arr_entries);
		}
		//printf("\n");

		QueueDestroy(&res_queue);
	}

	if(k == 0){
		for(i = 0;i < arr_entries;i++)
			free(freq_array[i].ngram);
		free(freq_array);				
		return;

	}

	/* Top k */

	/*Find max appearances that occur in the burst*/
	int max = -1;
	for(i = 0;i < arr_entries;i++)
		if(freq_array[i].times > max) max = freq_array[i].times;
	

	/*Each index of this array has a queue with every n-gram that has occured index-times*/
	Queue* reverse_array = calloc(max+1,sizeof(Queue));

	for(i = 0;i < arr_entries;i++){
		if(reverse_array[freq_array[i].times] == NULL) QueueCreate(&(reverse_array[freq_array[i].times]));
		QueuePush(reverse_array[freq_array[i].times],(void*)freq_array[i].ngram);
	}

	//printf("Top: ");
	int popped = 0;
	int next_pop = max;

	char* str = (char*)QueuePop(reverse_array[next_pop]);
	TEST_ASSERT_EQUAL_STRING(str,"My name is");

	popped++;

	if(QueueIsEmpty(reverse_array[next_pop])) next_pop--;
	while(reverse_array[next_pop] == NULL) next_pop--;

	while(popped < k && next_pop >= 1){
		str = (char*)QueuePop(reverse_array[next_pop]);
		if(popped == 1)		
			TEST_ASSERT_EQUAL_STRING(str,"My name is Kartoffel");
		if(popped == 2)		
			TEST_ASSERT_EQUAL_STRING(str,"My name is Slim Shady");
		if(QueueIsEmpty(reverse_array[next_pop])) next_pop--;
		while(reverse_array[next_pop] == NULL) next_pop--;
		popped++;
	}

	/*Free arrays*/
	for(i = 1;i < max+1;i++)
		if(reverse_array[i]) QueueDestroy(&(reverse_array[i]));

	free(reverse_array);

	for(i = 0;i < arr_entries;i++)
		free(freq_array[i].ngram);

	free(freq_array);

	while(res_queue = QueuePop(burst)){
		while((s = QueuePop(res_queue))) free(s);
		QueueDestroy(&res_queue);
	}
	QueueDestroy(&burst);
	DynamicTrieDestroy((void*)&trie);
}

void test_StaticTrieAdd_oneNgram()
{
	StaticTrie trie;
	StaticTrieCreate((void*)&trie, 10);

	char buffer[100];
	strcpy(buffer, "Hello how are you");

	StaticTrieAdd(trie, buffer);

	int x,y;
	HyperNode* t = StaticLinearHashLookup((StaticLinearHash)(trie->root->children), "Hello", &x, &y);

	TEST_ASSERT_EQUAL_STRING(t->word, "Hello");
	TEST_ASSERT_EQUAL_STRING(t->children[0].word, "how");
	TEST_ASSERT_EQUAL_STRING(t->children[0].children[0].word, "are");
	TEST_ASSERT_EQUAL_STRING(t->children[0].children[0].children[0].word, "you");
	
	TEST_ASSERT_EQUAL_INT(t->lengths_size,1);
	TEST_ASSERT_EQUAL_INT(t->children[0].lengths_size,1);
	TEST_ASSERT_EQUAL_INT(t->children[0].children[0].lengths_size,1);
	TEST_ASSERT_EQUAL_INT(t->children[0].children[0].children[0].lengths_size,1);
	
	TEST_ASSERT_EQUAL_INT(t->lengths[0],-5);
	TEST_ASSERT_EQUAL_INT(t->children[0].lengths[0],-3);
	TEST_ASSERT_EQUAL_INT(t->children[0].children[0].lengths[0],-3);
	TEST_ASSERT_EQUAL_INT(t->children[0].children[0].children[0].lengths[0],3);

	StaticTrieDestroy((void*)&trie);	 
}

void test_StaticTrieAdd_multipleNgrams()
{
	StaticTrie trie;
	StaticTrieCreate((void*)&trie, 10);

	char buffer[100];
	strcpy(buffer, "Hello how are you");
	StaticTrieAdd(trie, buffer);
	strcpy(buffer, "Hello how are they");
	StaticTrieAdd(trie, buffer);
	strcpy(buffer, "My name is Jeff");
	StaticTrieAdd(trie, buffer);

	int x,y;
	HyperNode* t = StaticLinearHashLookup((StaticLinearHash)(trie->root->children), "Hello", &x, &y);

	TEST_ASSERT_EQUAL_STRING(t->word, "Hello");
	TEST_ASSERT_EQUAL_STRING(t->children[0].word, "how");
	TEST_ASSERT_EQUAL_STRING(t->children[0].children[0].word, "are");
	TEST_ASSERT_EQUAL_STRING(t->children[0].children[0].children[0].word, "they");
	TEST_ASSERT_EQUAL_STRING(t->children[0].children[0].children[1].word, "you");

	TEST_ASSERT_EQUAL_INT(t->lengths_size,1);
	TEST_ASSERT_EQUAL_INT(t->children[0].lengths_size,1);
	TEST_ASSERT_EQUAL_INT(t->children[0].children[0].lengths_size,1);
	TEST_ASSERT_EQUAL_INT(t->children[0].children[0].children[1].lengths_size,1);
	TEST_ASSERT_EQUAL_INT(t->children[0].children[0].children[0].lengths_size,1);

	TEST_ASSERT_EQUAL_INT(t->lengths[0],-5);
	TEST_ASSERT_EQUAL_INT(t->children[0].lengths[0],-3);
	TEST_ASSERT_EQUAL_INT(t->children[0].children[0].lengths[0],-3);
	TEST_ASSERT_EQUAL_INT(t->children[0].children[0].children[1].lengths[0],3);
	TEST_ASSERT_EQUAL_INT(t->children[0].children[0].children[0].lengths[0],4);

	t = StaticLinearHashLookup((StaticLinearHash)(trie->root->children), "My", &x, &y);

	TEST_ASSERT_EQUAL_STRING(t->word, "My");
	TEST_ASSERT_EQUAL_STRING(t->children[0].word, "name");
	TEST_ASSERT_EQUAL_STRING(t->children[0].children[0].word, "is");
	TEST_ASSERT_EQUAL_STRING(t->children[0].children[0].children[0].word, "Jeff");
	
	TEST_ASSERT_EQUAL_INT(t->lengths_size,1);
	TEST_ASSERT_EQUAL_INT(t->children[0].lengths_size,1);
	TEST_ASSERT_EQUAL_INT(t->children[0].children[0].lengths_size,1);
	TEST_ASSERT_EQUAL_INT(t->children[0].children[0].children[0].lengths_size,1);

	TEST_ASSERT_EQUAL_INT(t->lengths[0],-2);
	TEST_ASSERT_EQUAL_INT(t->children[0].lengths[0],-4);
	TEST_ASSERT_EQUAL_INT(t->children[0].children[0].lengths[0],-2);
	TEST_ASSERT_EQUAL_INT(t->children[0].children[0].children[0].lengths[0],4);
	
	StaticTrieDestroy((void*)&trie);
}

void test_StaticTrieCompress()
{
	StaticTrie trie;
	StaticTrieCreate((void*)&trie, 10);

	char buffer[100];
	
	strcpy(buffer, "Hello how are");
	StaticTrieAdd(trie, buffer);
	
	strcpy(buffer, "Hello how are you my friend");
	StaticTrieAdd(trie, buffer);
	
	strcpy(buffer, "Hello how are they my friend");
	StaticTrieAdd(trie, buffer);

	strcpy(buffer, "Hello how are they hi boy");
	StaticTrieAdd(trie, buffer);

	StaticTrieCompress(trie);

	int x,y;
	HyperNode* t = StaticLinearHashLookup((StaticLinearHash)(trie->root->children), "Hello", &x, &y);

	TEST_ASSERT_EQUAL_STRING(t->word, "Hellohoware");
	TEST_ASSERT_EQUAL_INT(t->lengths_size,3);
	TEST_ASSERT_EQUAL_INT(t->lengths[0],-5);
	TEST_ASSERT_EQUAL_INT(t->lengths[1],-3);
	TEST_ASSERT_EQUAL_INT(t->lengths[2],3);

	TEST_ASSERT_EQUAL_STRING(t->children[0].word, "they");
	TEST_ASSERT_EQUAL_INT(t->children[0].lengths_size,1);
	TEST_ASSERT_EQUAL_INT(t->children[0].lengths[0],-4);
	TEST_ASSERT_EQUAL_STRING(t->children[1].word, "youmyfriend");
	TEST_ASSERT_EQUAL_INT(t->children[1].lengths_size,3);
	TEST_ASSERT_EQUAL_INT(t->children[1].lengths[0],-3);
	TEST_ASSERT_EQUAL_INT(t->children[1].lengths[1],-2);
	TEST_ASSERT_EQUAL_INT(t->children[1].lengths[2],6);
	
	TEST_ASSERT_EQUAL_STRING(t->children[0].children[0].word,"hiboy");
	TEST_ASSERT_EQUAL_INT(t->children[0].children[0].lengths_size,2);
	TEST_ASSERT_EQUAL_INT(t->children[0].children[0].lengths[0],-2);
	TEST_ASSERT_EQUAL_INT(t->children[0].children[0].lengths[1],3);

	TEST_ASSERT_EQUAL_STRING(t->children[0].children[1].word,"myfriend");
	TEST_ASSERT_EQUAL_INT(t->children[0].children[1].lengths_size,2);
	TEST_ASSERT_EQUAL_INT(t->children[0].children[1].lengths[0],-2);
	TEST_ASSERT_EQUAL_INT(t->children[0].children[1].lengths[1],6);

	StaticTrieDestroy((void*)&trie);
}

void test_StaticTrieQuestion()
{
	StaticTrie trie;
	StaticTrieCreate((void*)&trie, 10);
	char* s;
	char buffer[100];
	
	strcpy(buffer, "Hello how are");
	StaticTrieAdd(trie, buffer);
	
	strcpy(buffer, "Hello how are you my friend");
	StaticTrieAdd(trie, buffer);
	
	strcpy(buffer, "Hello how are they my friend");
	StaticTrieAdd(trie, buffer);

	strcpy(buffer, "Hello how are they hi boy");
	StaticTrieAdd(trie, buffer);

	StaticTrieCompress(trie);

	Queue q;

	strcpy(buffer, "Hello how are");
	q = StaticTrieQuestion(trie, buffer);
	TEST_ASSERT_EQUAL_STRING("Hello how are", s = (char*)QueuePop(q));
	free(s);
	QueueDestroy(&q);

	strcpy(buffer, "Here is me and hello	Hello how are you my friend");
	q = StaticTrieQuestion(trie, buffer);
	TEST_ASSERT_EQUAL_STRING("Hello how are", s = (char*)QueuePop(q));
	free(s);
	TEST_ASSERT_EQUAL_STRING("Hello how are you my friend", s = (char*)QueuePop(q));
	free(s);
	QueueDestroy(&q);

	StaticTrieDestroy((void*)&trie);
	
}

int main()
{
	UNITY_BEGIN();

	/* RUN TESTS */
	RUN_TEST(test_DynamicTrieAdd_oneNgram);
	RUN_TEST(test_DynamicTrieAdd_multipleNgrams);
	RUN_TEST(test_DynamicTrieAdd_AlreadyExists);
	RUN_TEST(test_TrieAdd_InvokeArrayRealloc);
	RUN_TEST(test_DynamicTrieAdd_WhiteSpace);
	RUN_TEST(test_DynamicTrie_QuestionAndDelete);
	RUN_TEST(test_DynamicTrie_DeleteFails);
	RUN_TEST(test_BloomFilter);
	RUN_TEST(test_TopK);

	RUN_TEST(test_StaticTrieAdd_oneNgram);
	RUN_TEST(test_StaticTrieAdd_multipleNgrams);
	RUN_TEST(test_StaticTrieCompress);
	RUN_TEST(test_StaticTrieQuestion);

	return UNITY_END();
}
