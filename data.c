#include "data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static FILE *openFile(const char *filepath, const char *mode) {
    FILE *fp = fopen(filepath, mode);
    if (!fp) fprintf(stderr, "[data] Cannot open file: %s\n", filepath);
    return fp;
}

static inline long long fastParseLl(const char **p) {
    while (**p == ' ' || **p == '\t') (*p)++;
    long long v = 0;
    while ((unsigned int)((unsigned char)**p - '0') <= 9u) {
        v = v * 10 + (*(*p)++ - '0');
    }
    return v;
}

WordData *loadVocab(const char *filepath, size_t *outCount) {
    *outCount = 0;
    FILE *fp = openFile(filepath, "r");
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
        data[idx].totalFreq = 0LL;
        idx++;
    }

    fclose(fp);
    *outCount = idx;
    return data;
}

int accumulateFrequencies(const char *filepath, WordData *data, size_t wordCount,
                          size_t *outD, size_t *outW, long long *outN) {
    *outD = *outW = 0; *outN = 0LL;

    if (!data || wordCount == 0) return -1;

    FILE *fp = openFile(filepath, "rb");
    if (!fp) return -1;

    char hdr[MAX_LINE_LEN];
    if (!fgets(hdr, sizeof(hdr), fp)) { fclose(fp); return -1; }
    *outD = (size_t)strtoull(hdr, NULL, 10);

    if (!fgets(hdr, sizeof(hdr), fp)) { fclose(fp); return -1; }
    *outW = (size_t)strtoull(hdr, NULL, 10);

    if (!fgets(hdr, sizeof(hdr), fp)) { fclose(fp); return -1; }
    *outN = strtoll(hdr, NULL, 10);

    // Alokasi buffer
    char *buf = malloc(IO_BLOCK_SIZE + MAX_LINE_LEN + 1);
    if (!buf) { fclose(fp); return -1; }

    size_t leftover = 0;
    size_t rd;
    size_t spaceAvailable = IO_BLOCK_SIZE + MAX_LINE_LEN;
    size_t toRead = IO_BLOCK_SIZE;

    while ((rd = fread(buf + leftover, 1, toRead, fp)) > 0 || leftover > 0) {
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
            (void)fastParseLl(&lp); // Skip docID
            long long wordId = fastParseLl(&lp);
            long long count = fastParseLl(&lp);

            // Validasi boundary ketat
            if (wordId > 0 && (size_t)wordId <= wordCount) {
                data[wordId - 1].totalFreq += count;
            }

            p = (nl == end) ? end : nl + 1;
        }

        if (rd == 0 && leftover == 0) break;

        spaceAvailable = IO_BLOCK_SIZE + MAX_LINE_LEN - leftover;
        toRead = (spaceAvailable > IO_BLOCK_SIZE) ? IO_BLOCK_SIZE : spaceAvailable;
    }

    free(buf);
    fclose(fp);
    return 0;
}

void compactWordData(WordData **data, size_t *wordCount) {
    if (!data || !*data || *wordCount == 0) return;

    size_t writeIdx = 0;
    for (size_t i = 0; i < *wordCount; i++) {
        if ((*data)[i].totalFreq > 0) {
            if (i != writeIdx) {
                (*data)[writeIdx] = (*data)[i];
                (*data)[i].word = NULL;
            }
            writeIdx++;
        } else {
            free((*data)[i].word);
            (*data)[i].word = NULL;
        }
    }

    // Antisipasi jika tidak ada kata yang valid sama sekali
    if (writeIdx == 0) {
        free(*data);
        *data = NULL;
    } else if (writeIdx < *wordCount) {
        WordData *shrunk = realloc(*data, writeIdx * sizeof(WordData));
        if (shrunk) {
            *data = shrunk;
        }
    }

    *wordCount = writeIdx;
}

void freeWordData(WordData **data, size_t *wordCount) {
    if (!data || !*data) return;

    for (size_t i = 0; i < *wordCount; i++) {
        free((*data)[i].word);
        (*data)[i].word = NULL;
    }

    free(*data);
    *data = NULL;
    *wordCount = 0;
}
