#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bag_of_words.h"
#include "sorting.h"
#include "output.h"

void print_menu() {
    printf("\nPilihan:\n");
    printf("1) Urutkan Data Menggunakan Insertion Sort & Simpan ke File.\n");
    printf("2) Urutkan Data Menggunakan Quicksort & Simpan ke File.\n");
    printf("3) Urutkan Data Menggunakan Heapsort & Simpan ke File.\n");
    printf("4) Tampilkan ke Layar Top-K Kata Dengan Frekuensi Terbesar\n");
    printf("5) Selesai\n");
    printf("\nPilihan anda: ");
}

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main(void) {
    char docword_file[256];
    char vocab_file[256] = {0};

    // 1. Meminta input dari user sesuai petunjuk PDF
    printf("Tentukan file docword: ");
    if (scanf("%255s", docword_file) != 1) {
        fprintf(stderr, "Gagal membaca input file.\n");
        return EXIT_FAILURE;
    }

    // Fitur Cerdas: Otomatis mencari nama file vocab berdasarkan input docword
    // Contoh: "docword.kos.txt" -> "vocab.kos.txt"
    char *ptr = strstr(docword_file, "docword");
    if (ptr != NULL) {
        int prefix_len = ptr - docword_file;
        strncpy(vocab_file, docword_file, prefix_len);
        strcat(vocab_file, "vocab");
        strcat(vocab_file, ptr + strlen("docword"));
    } else {
        // Fallback jika nama file aneh
        printf("Tentukan file vocabulary: ");
        scanf("%255s", vocab_file);
    }

    int W = 0;
    printf("\nMemuat data dari '%s' dan '%s'...\n", docword_file, vocab_file);
    
    WordData *original_data = load_bow_data(vocab_file, docword_file, &W);
    if (!original_data) {
        return EXIT_FAILURE;
    }

    WordData *sorted_data = malloc((size_t)W * sizeof(WordData));
    if (!sorted_data) {
        fprintf(stderr, "Error: Gagal alokasi memori untuk buffer sorting.\n");
        free(original_data);
        return EXIT_FAILURE;
    }

    int choice;
    int is_sorted = 0;
    double last_elapsed_ms = 0.0;

    do {
        print_menu();
        if (scanf("%d", &choice) != 1) {
            clear_input_buffer();
            printf("\n[ERROR] Input tidak valid!\n");
            continue;
        }

        if (choice >= 1 && choice <= 3) {
            char output_file[256];
            printf("Tentukan nama file output (contoh: hasil.txt): ");
            scanf("%255s", output_file);

            // Copy ulang data agar selalu mengurutkan dari data mentah
            memcpy(sorted_data, original_data, (size_t)W * sizeof(WordData));
            
            clock_t start_time, end_time;
            printf("Mengurutkan data... (Harap tunggu)\n");

            if (choice == 1) {
                start_time = clock();
                insertionSort(sorted_data, W);
                end_time = clock();
            } else if (choice == 2) {
                start_time = clock();
                quickSort(sorted_data, 0, W - 1);
                end_time = clock();
            } else {
                start_time = clock();
                heapSort(sorted_data, W);
                end_time = clock();
            }

            last_elapsed_ms = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000.0;
            is_sorted = 1;

            saveToFile(output_file, sorted_data, W, last_elapsed_ms);
            printf("Data berhasil diurutkan dan disimpan ke '%s' dalam waktu %.3f ms.\n", output_file, last_elapsed_ms);

        } else if (choice == 4) {
            int k;
            printf("Masukkan nilai k (jumlah kata terbesar yang ingin ditampilkan): ");
            if (scanf("%d", &k) != 1) {
                clear_input_buffer();
                continue;
            }

            if (!is_sorted) {
                // Jika user langsung pilih nomor 4 tanpa sorting, paksa pakai Quicksort
                printf("Data belum diurutkan. Otomatis mengurutkan menggunakan Quicksort...\n");
                memcpy(sorted_data, original_data, (size_t)W * sizeof(WordData));
                clock_t start_time = clock();
                quickSort(sorted_data, 0, W - 1);
                clock_t end_time = clock();
                last_elapsed_ms = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000.0;
                is_sorted = 1;
            }

            displayTopK(sorted_data, W, k, last_elapsed_ms);

        } else if (choice == 5) {
            printf("\nProgram selesai.\n");
        } else {
            printf("\nPilihan tidak valid.\n");
        }

    } while (choice != 5);

    free(original_data);
    free(sorted_data);
    
    return EXIT_SUCCESS;
}