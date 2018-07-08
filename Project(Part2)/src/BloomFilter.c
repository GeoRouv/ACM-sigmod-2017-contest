#include <stdlib.h>
#include <string.h>

#include "BloomFilter.h"
#include "MurmurHash2.h"

typedef struct BloomFilter_t {

	Bool* array;
	int size;
	int numHashFunctions;

} BloomFilter_t;

void BloomFilterCreate(BloomFilter* bf)
{
	(*bf) = malloc(sizeof(BloomFilter_t));
	(*bf)->array = calloc(M, sizeof(Bool));
	(*bf)->size = M;
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
	memset(bf->array, 0, bf->size);
}

Bool BloomFilterAsk(BloomFilter bf, char* key)
{
	int i;
	int sum = 0;
	for(i = 1; i <= bf->numHashFunctions; i++)
	{
		int pos = MurmurHash2(key, strlen(key), i*9417401238) % bf->size;
		if(bf->array[pos] == 0)
			bf->array[pos] = 1;
		else sum++;
	}

	return (bf->numHashFunctions == sum) ? 0:1;
}