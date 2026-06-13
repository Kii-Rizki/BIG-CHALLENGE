#ifndef BAG_OF_WORDS_H
#define BAG_OF_WORDS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char word[100];
    uint64_t total_frequency;
} WordData;

WordData *load_bow_data(const char *vocab_path, const char *docword_path, int *out_W);

#endif