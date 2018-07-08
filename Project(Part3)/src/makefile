CFLAGS = -g3 -O2

ngrams: main.o DynamicTrie.o StaticTrie.o BloomFilter.o Queue.o MurmurHash2.o AssistingFunctions.o LinearHash.o StaticLinearHash.o JobScheduler.o
	gcc $(CFLAGS) -pthread main.o DynamicTrie.o StaticTrie.o BloomFilter.o Queue.o MurmurHash2.o AssistingFunctions.o LinearHash.o StaticLinearHash.o JobScheduler.o -o ngrams 

main.o: main.c
	gcc $(CFLAGS) -c main.c

DynamicTrie.o: DynamicTrie.c DynamicTrie.h
	gcc $(CFLAGS) -c DynamicTrie.c

StaticTrie.o: StaticTrie.c StaticTrie.h
	gcc $(CFLAGS) -c StaticTrie.c

BloomFilter.o: BloomFilter.c BloomFilter.h
	gcc $(CFLAGS) -c BloomFilter.c

Queue.o: Queue.c Queue.h
	gcc $(CFLAGS) -c Queue.c

AssistingFunctions.o: AssistingFunctions.c AssistingFunctions.h
	gcc $(CFLAGS) -c AssistingFunctions.c

MurmurHash2.o: MurmurHash2.c MurmurHash2.h
	gcc $(CFLAGS) -c MurmurHash2.c

LinearHash.o: LinearHash.c LinearHash.h
	gcc $(CFLAGS) -c LinearHash.c

StaticLinearHash.o: StaticLinearHash.c StaticLinearHash.h
	gcc $(CFLAGS) -c StaticLinearHash.c

JobScheduler.o: JobScheduler.c JobScheduler.h
	gcc $(CFLAGS) -pthread -c JobScheduler.c

execute_small_dynamic:
	time ./ngrams -i ../small/small_dynamic/small_dynamic.init -q ../small/small_dynamic/small_dynamic.work > out.txt
	
execute_small_static:
	time ./ngrams -i ../small/small_static/small_static.init -q ../small/small_static/small_static.work > out.txt

execute_medium_dynamic:
	time ./ngrams -i ../medium/medium_dynamic/medium_dynamic.init -q ../medium/medium_dynamic/medium_dynamic.work > out.txt

execute_medium_static:
	time ./ngrams -i ../medium/medium_static/medium_static.init -q ../medium/medium_static/medium_static.work > out.txt

clean:
	rm *.o
	rm ngrams
	rm ./out.txt
