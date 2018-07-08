#include <stdlib.h>
#include <string.h>

#include "BloomFilter.h"
#include "MurmurHash2.h"

typedef struct BloomFilter_t {

	int* array;
	int size;
	int numHashFunctions;

} BloomFilter_t;

void BloomFilterCreate(BloomFilter* bf)
{
	(*bf) = malloc(sizeof(BloomFilter_t));
	(*bf)->array = calloc(M, sizeof(int));
	(*bf)->size = M*8;
	(*bf)->numHashFunctions = K;
}

void BloomFilterDestroy(BloomFilter* bf)
{
	free((*bf)->array);
	free(*bf);
	*bf = NULL;
}

void BloomFilterReset(BloomFilter bf)
{
	memset(bf->array, 0, M);
}

Bool BloomFilterAsk(BloomFilter bf, char* key)
{
	int i;
	int sum = 0;
	for(i = 1; i <= bf->numHashFunctions; i++)
	{
		int pos = MurmurHash2(key, strlen(key), i*9417401238) % bf->size;
		if(!TestBit(bf->array,pos))
			SetBit(bf->array,pos);
		else sum++;
	}

	return (bf->numHashFunctions == sum) ? 0:1;
}