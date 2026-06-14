#ifndef OUTPUT_H
#define OUTPUT_H

#include "bag_of_words.h"

void saveToFile(
    const char *filename,
    WordData data[],
    int n,
    double elapsed_ms);

void displayTopK(
    WordData data[],
    int n,
    int k,
    double elapsed_ms);

#endif
