#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static unsigned int hits = 0;
static unsigned int misses = 0;
static unsigned int evictions = 0;

typedef struct {
	unsigned int validBit; // 1 or 0
	unsigned long int tag;
	unsigned int LRU; // least recently used
} CacheLine;

void simulation(CacheLine ** cache, unsigned long int setIndex, unsigned long int tag, unsigned int S, unsigned int E) {
	int match = 0; int addIndex = -1;
	CacheLine * cacheSet = cache[setIndex];
	for (int i = 0; i < E; i++) {
		CacheLine cacheLine = cacheSet[i]; // technically you don't need this cacheLine variable and can just use cache[setIndex][i] for everything (same goes for the cacheSet variable above) 
		if (cacheLine.validBit == 1) {
			// hit
			if (cacheLine.tag == tag) {
				match = 1;
				hits++;
				cache[setIndex][i].LRU = 0;
				break;
			}
			// valid bit - tag doesn't match
			else {
				cache[setIndex][i].LRU += 1;
			}
		}
		else {
			addIndex = i;
		}
	}
	// misses
	if (match == 0) {
		// miss - no eviction
		if (addIndex != -1) {
			CacheLine addLine = {1, tag, 0}; // block == line
			cache[setIndex][addIndex] = addLine;
			misses++;
		}
		// miss - do evict
		else {
			int max = 0; int evictIndex = -1;
			for (int i = 0; i < E; i++) {
				if (cache[setIndex][i].validBit == 1 && cache[setIndex][i].LRU >= max) {
					max = cache[setIndex][i].LRU;
					evictIndex = i;
				}
			}
			CacheLine replaceLine = {1, tag, 0};
			cache[setIndex][evictIndex] = replaceLine;
			misses++;
			evictions++;
		}
	}
}

void readTo(char fileContents[], unsigned int b, unsigned int s, unsigned int S, unsigned int E, CacheLine ** cache) {
	FILE * file1 = fopen(fileContents, "r");
	char operation; unsigned long int address; int size;
	// [SPACE] operation address,size
	while (fscanf(file1, " %c %lx,%d", &operation, &address, &size) != EOF) {
		// instruction load
		if (operation == 'I') {
			continue;
		}
		// data modify
		if (operation == 'M') {
			hits++;
		}
		
		/* 
		1.) tag
		2.) valid bit
		we have a hit
		*/
		unsigned long int tag = address >> s >> b; // tag - set - block offset
		unsigned long int setIndex = address >> b & (S - 1); // adddress >> block offset & (cache size - 1)
		simulation(cache, setIndex, tag, S, E);
	}
	fclose(file1);
}

// malloc = memory allocation
CacheLine ** createCache(unsigned int S, unsigned int E) {
	CacheLine ** cache = (CacheLine**)malloc(S * sizeof(CacheLine*)); // rows
	for (int i = 0; i < S; i++) {
		cache[i] = (CacheLine*)malloc(E * sizeof(CacheLine)); // columns
	}
	return cache;
}

void freeCache(CacheLine ** cache, unsigned int S) {
	for (int i = 0; i < S; i++) {
		free(cache[i]);
	}
	free(cache);
}

int main(int argc, char ** argv)
{
	unsigned int s = 0; // number of bits in set address
	unsigned int E = 0; // number of cache blocks in a set
	unsigned int b = 0; // number of bits in a cache block
	char fileContents[1000] = "";
	
	int initializer = 0;
	while ((initializer = getopt(argc, argv, "s:E:b:t:")) != -1) {
		switch (initializer) {
			case 's':
				s = atoi(optarg);
				break;
			case 'E':
				E = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
				break;
			case 't':
				strcpy(fileContents, optarg);
				break;
			default:
				break;
		}
	}
	
	unsigned int S = 1 << s; // number of sets in cache
	CacheLine ** cache = createCache(S, E);
	readTo(fileContents, b, s, S, E, cache);
	freeCache(cache, S);
	printSummary(hits, misses, evictions);
	return 0;
}
