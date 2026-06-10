#ifndef DATA_H
#define DATA_H

#include <stddef.h>

#define MAX_WORD_LEN 256
#define IO_BLOCK_SIZE (1 << 22)
#define MAX_LINE_LEN 128

typedef struct {
    char *word;
    long long totalFreq;
} WordData;

WordData *loadVocab(const char *filepath, size_t *outCount);

int accumulateFrequencies(const char *filepath, WordData *data, size_t wordCount,
                          size_t *outD, size_t *outW, long long *outN);

// Menggunakan pointer-to-pointer agar aman saat realloc
void compactWordData(WordData **data, size_t *wordCount);

void freeWordData(WordData **data, size_t *wordCount);

#endif
