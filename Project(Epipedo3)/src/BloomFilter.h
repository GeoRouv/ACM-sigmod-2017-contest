#ifndef __BLOOMFILTER_H__
#define __BLOOMFILTER_H__

/*

Project team:
1) Stefou Theodoros 		1115201400193
2) Kokkinakos Panagiotis	1115201400069
3) Rouvalis Georgios		1115201400173

*/

#define SetBit(A,k)     ( A[(k/32)] |= (1 << (k%32)) )
#define ClearBit(A,k)   ( A[(k/32)] &= ~(1 << (k%32)) )
#define TestBit(A,k)    ( A[(k/32)] & (1 << (k%32)) )

/* Small dataset */
/*#define M 1160
#define K 4*/

/* Medium and Large datasets */
#define M 7200
#define K 5

typedef char Bool;

typedef struct BloomFilter_t* BloomFilter;

void BloomFilterCreate(BloomFilter* bf);
void BloomFilterReset(BloomFilter bf);
Bool BloomFilterAsk(BloomFilter bf, char* key);
void BloomFilterDestroy(BloomFilter* bf);

#endif
