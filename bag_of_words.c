#include "bag_of_words.h"

#define LINE_BUFFER_SIZE 128
#define MAX_WORD_LEN 100

WordData *load_bow_data(const char *vocab_path, const char *docword_path, int *out_W)
{

    FILE *fdoc = fopen(docword_path, "r");
    if (fdoc == NULL) {
        fprintf(stderr, "Error: gagal membuka '%s'\n", docword_path);
        exit(EXIT_FAILURE);
    }

    int D, W, N;

    // Baca D
    if (fscanf(fdoc, "%d", &D) != 1) {
        fprintf(stderr, "Error: gagal membaca nilai D\n");
        fclose(fdoc);
        exit(EXIT_FAILURE);
    }
    if (D <= 0) {
        fprintf(stderr, "Error: nilai D tidak valid (D=%d)\n", D);
        fclose(fdoc);
        exit(EXIT_FAILURE);
    }

    // Baca W
    if (fscanf(fdoc, "%d", &W) != 1) {
        fprintf(stderr, "Error: gagal membaca nilai W\n");
        fclose(fdoc);
        exit(EXIT_FAILURE);
    }
    if (W <= 0) {
        fprintf(stderr, "Error: nilai W tidak valid (W=%d)\n", W);
        fclose(fdoc);
        exit(EXIT_FAILURE);
    }

    // Baca N
    if (fscanf(fdoc, "%d", &N) != 1) {
        fprintf(stderr, "Error: gagal membaca nilai N\n");
        fclose(fdoc);
        exit(EXIT_FAILURE);
    }
    if (N <= 0) {
        fprintf(stderr, "Error: nilai N tidak valid (N=%d)\n", N);
        fclose(fdoc);
        exit(EXIT_FAILURE);
    }

    {
        int ch;
        while ((ch = fgetc(fdoc)) != '\n' && ch != EOF) {
        }
    }

    WordData *data = calloc((size_t)W, sizeof(WordData));
    if (data == NULL) {
        fprintf(stderr, "Error: calloc gagal untuk %d elemen WordData\n", W);
        fclose(fdoc);
        exit(EXIT_FAILURE);
    }

    FILE *fvoc = fopen(vocab_path, "r");
    if (fvoc == NULL) {
        fprintf(stderr, "Error: gagal membuka '%s'\n", vocab_path);
        free(data);
        fclose(fdoc);
        exit(EXIT_FAILURE);
    }

    char line[MAX_WORD_LEN];
    int  word_index = 0;

    while (fgets(line, sizeof(line), fvoc) != NULL) {
        if (word_index >= W) {
            fprintf(stderr, "Warning: vocab melebihi W=%d, pembacaan dihentikan\n", W);
            break;
        }

        line[strcspn(line, "\r\n")] = '\0';

        if (line[0] == '\0') {
            continue;
        }

        strncpy(data[word_index].word, line, MAX_WORD_LEN);
        data[word_index].word[MAX_WORD_LEN - 1] = '\0';

        word_index++;
    }

    fclose(fvoc);

    char buf[LINE_BUFFER_SIZE];

    for (int i = 0; i < N; i++) {
        if (fgets(buf, sizeof(buf), fdoc) == NULL) {
            fprintf(stderr, "Error: gagal membaca baris ke-%d\n", i + 1);
            free(data);
            fclose(fdoc);
            exit(EXIT_FAILURE);
        }

        char *ptr    = buf;
        char *endptr = NULL;

        (void)strtoul(ptr, &endptr, 10);
        if (endptr == ptr) {
            goto parse_error;
        }
        ptr = endptr;

        while (*ptr == ' ') {
            ptr++;
        }

        unsigned long wordID = strtoul(ptr, &endptr, 10);
        if (endptr == ptr) {
            goto parse_error;
        }
        ptr = endptr;

        while (*ptr == ' ') {
            ptr++;
        }

        unsigned long long count = strtoull(ptr, &endptr, 10);
        if (endptr == ptr) {
            goto parse_error;
        }

        if (wordID < 1UL || wordID > (unsigned long)W) {
            fprintf(stderr,
                    "Error: wordID=%lu di luar rentang [1,%d] pada iterasi %d\n",
                    wordID, W, i + 1);
            free(data);
            fclose(fdoc);
            exit(EXIT_FAILURE);
        }

        data[wordID - 1].total_frequency += (uint64_t)count;
        continue;

parse_error:
        fprintf(stderr, "Error: parsing gagal pada iterasi %d\n", i + 1);
        free(data);
        fclose(fdoc);
        exit(EXIT_FAILURE);
    }

    fclose(fdoc);

    *out_W = W;
    return data;
}
