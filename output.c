#include "output.h"

void saveToFile(
    const char *filename,
    WordData data[],
    int n,
    double elapsed_ms)
{
    FILE *fp = fopen(filename, "w");

    if (!fp) {
        printf("Gagal membuat file output\n");
        return;
    }

    for (int i = 0; i < n; i++) {
        fprintf(fp,
                "%s (%llu)\n",
                data[i].word,
                (unsigned long long)data[i].total_frequency);
    }

    fprintf(fp,
            "\nWaktu untuk mengurutkan : %.3f ms\n",
            elapsed_ms);

    fclose(fp);
}

void displayTopK(
    WordData data[],
    int n,
    int k,
    double elapsed_ms)
{
    if (k > n)
        k = n;

    printf("\nTOP %d KATA\n\n", k);

    for (int i = 0; i < k; i++) {
        printf("%s (%llu)\n",
               data[i].word,
               (unsigned long long)data[i].total_frequency);
    }

    printf("\nWaktu sorting : %.3f ms\n",
           elapsed_ms);
}
