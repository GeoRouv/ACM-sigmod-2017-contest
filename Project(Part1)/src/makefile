ngrams: main.o Trie.o Queue.o
	gcc -g3 -O2 main.o Trie.o Queue.o -o ngrams 

main.o: main.c
	gcc -g3 -O2 -c main.c

Trie.o: Trie.c Trie.h
	gcc -g3 -O2 -c Trie.c

Queue.o: Queue.c Queue.h
	gcc -g3 -O2 -c Queue.c

clean:
	rm *.o
	rm ngrams