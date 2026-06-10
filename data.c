#include "data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static FILE *open_file(const char *filepath, const char *mode) {
    FILE *fp = fopen(filepath, mode);
    if (!fp) fprintf(stderr, "[data] Cannot open file: %s\n", filepath);
    return fp;
}

static inline long long fast_parse_ll(const char **p) {
    while (**p == ' ' || **p == '\t') (*p)++;
    long long v = 0;
    while ((unsigned int)((unsigned char)**p - '0') <= 9u) {
        v = v * 10 + (*(*p)++ - '0');
    }
    return v;
}

WordData *load_vocab(const char *filepath, size_t *out_count) {
    *out_count = 0;
    FILE *fp = open_file(filepath, "r");
    if (!fp) return NULL;

    size_t capacity = 0;
    char line[MAX_WORD_LEN + 2];

    // Hitung baris dengan proteksi baris terlalu panjang
    while (fgets(line, sizeof(line), fp)) {
        capacity++;
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] != '\n') {
            int ch;
            while ((ch = fgetc(fp)) != '\n' && ch != EOF); // Buang sisa baris
        }
    }

    if (capacity == 0) {
        fclose(fp);
        return NULL;
    }

    WordData *data = calloc(capacity, sizeof(WordData));
    if (!data) {
        fclose(fp);
        return NULL;
    }

    // Mengisi data kata
    rewind(fp);
    size_t idx = 0;

    while (idx < capacity && fgets(line, sizeof(line), fp)) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] != '\n') {
            int ch;
            while ((ch = fgetc(fp)) != '\n' && ch != EOF); // Sinkronisasi buang sisa
        }

        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }

        data[idx].word = malloc(len + 1);
        if (!data[idx].word) {
            for (size_t i = 0; i < idx; i++) free(data[i].word);
            free(data);
            fclose(fp);
            return NULL;
        }

        memcpy(data[idx].word, line, len + 1);
        data[idx].total_freq = 0LL;
        idx++;
    }

    fclose(fp);
    *out_count = idx;
    return data;
}

int accumulate_frequencies(const char *filepath, WordData *data, size_t word_count,
                           size_t *out_D, size_t *out_W, long long *out_N) {
    *out_D = *out_W = 0; *out_N = 0LL;

    if (!data || word_count == 0) return -1;

    FILE *fp = open_file(filepath, "rb");
    if (!fp) return -1;

    char hdr[MAX_LINE_LEN];
    if (!fgets(hdr, sizeof(hdr), fp)) { fclose(fp); return -1; }
    *out_D = (size_t)strtoull(hdr, NULL, 10);

    if (!fgets(hdr, sizeof(hdr), fp)) { fclose(fp); return -1; }
    *out_W = (size_t)strtoull(hdr, NULL, 10);

    if (!fgets(hdr, sizeof(hdr), fp)) { fclose(fp); return -1; }
    *out_N = strtoll(hdr, NULL, 10);

    // Alokasi buffer
    char *buf = malloc(IO_BLOCK_SIZE + MAX_LINE_LEN + 1);
    if (!buf) { fclose(fp); return -1; }

    size_t leftover = 0;
    size_t rd;
    size_t space_available = IO_BLOCK_SIZE + MAX_LINE_LEN;
    size_t to_read = IO_BLOCK_SIZE;

    while ((rd = fread(buf + leftover, 1, to_read, fp)) > 0 || leftover > 0) {
        // Antisipasi Over-Read
        buf[leftover + rd] = '\0';
        
        const char *p = buf;
        const char *end = buf + leftover + rd;
        leftover = 0;

        while (p < end) {
            const char *nl = (const char *)memchr(p, '\n', (size_t)(end - p));

            if (!nl) {
                if (rd == 0) {
                    nl = end;
                } else {
                    leftover = (size_t)(end - p);
                    if (leftover > MAX_LINE_LEN) {
                        fprintf(stderr, "[data] Baris terlalu panjang atau korup.\n");
                        free(buf); fclose(fp); return -1;
                    }
                    memmove(buf, p, leftover);
                    break;
                }
            }

            const char *lp = p;
            (void)fast_parse_ll(&lp); // Skip docID
            long long word_id = fast_parse_ll(&lp);
            long long count = fast_parse_ll(&lp);

            // Validasi boundary ketat
            if (word_id > 0 && (size_t)word_id <= word_count) {
                data[word_id - 1].total_freq += count;
            }

            p = (nl == end) ? end : nl + 1;
        }

        if (rd == 0 && leftover == 0) break;
        
        space_available = IO_BLOCK_SIZE + MAX_LINE_LEN - leftover;
        to_read = (space_available > IO_BLOCK_SIZE) ? IO_BLOCK_SIZE : space_available;
    }

    free(buf);
    fclose(fp);
    return 0;
}

void compact_word_data(WordData **data, size_t *word_count) {
    if (!data || !*data || *word_count == 0) return;
    
    size_t write_idx = 0;
    for (size_t i = 0; i < *word_count; i++) {
        if ((*data)[i].total_freq > 0) {
            if (i != write_idx) {
                (*data)[write_idx] = (*data)[i];
                (*data)[i].word = NULL;
            }
            write_idx++;
        } else {
            free((*data)[i].word);
            (*data)[i].word = NULL;
        }
    }

    // Antisipasi jika tidak ada kata yang valid sama sekali
    if (write_idx == 0) {
        free(*data);
        *data = NULL;
    } else if (write_idx < *word_count) {
        WordData *shrunk = realloc(*data, write_idx * sizeof(WordData));
        if (shrunk) {
            *data = shrunk;
        }
    }
    
    *word_count = write_idx;
}

void free_word_data(WordData **data, size_t *word_count) {
    if (!data || !*data) return;

    for (size_t i = 0; i < *word_count; i++) {
        free((*data)[i].word);
        (*data)[i].word = NULL;
    }

    free(*data);
    *data = NULL;
    *word_count = 0;
}