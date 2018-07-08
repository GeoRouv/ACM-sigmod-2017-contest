#ifndef __BLOOMFILTER_H__
#define __BLOOMFILTER_H__

/*

Project team:
1) Stefou Theodoros 		1115201400193
2) Kokkinakos Panagiotis	1115201400069
3) Rouvalis Georgios		1115201400173

*/

/* Small dataset */
/*#define M 9500
#define K 4*/

/* Medium dataset */
#define M 57500 
#define K 5

typedef char Bool;

typedef struct BloomFilter_t* BloomFilter;

void BloomFilterCreate(BloomFilter* bf);
void BloomFilterReset(BloomFilter bf);
Bool BloomFilterAsk(BloomFilter bf, char* key);
void BloomFilterDestroy(BloomFilter* bf);

#endif
