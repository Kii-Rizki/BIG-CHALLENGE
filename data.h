#ifndef DATA_H
#define DATA_H

#include <stddef.h>

#define MAX_WORD_LEN 256
#define IO_BLOCK_SIZE (1 << 22)
#define MAX_LINE_LEN 128

typedef struct {
    char *word;
    long long total_freq;
} WordData;

WordData *load_vocab(const char *filepath, size_t *out_count);

int accumulate_frequencies(const char *filepath, WordData *data, size_t word_count,
                           size_t *out_D, size_t *out_W, long long *out_N);

// Menggunakan pointer-to-pointer agar aman saat realloc
void compact_word_data(WordData **data, size_t *word_count);

void free_word_data(WordData **data, size_t *word_count);

#endif