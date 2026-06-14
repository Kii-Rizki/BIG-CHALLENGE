#include "sorting.h"

static void swap(WordData *a, WordData *b)
{
    WordData temp = *a;
    *a = *b;
    *b = temp;
}

void insertionSort(WordData arr[], int n)
{
    for (int i = 1; i < n; i++) {
        WordData key = arr[i];
        int j = i - 1;

        while (j >= 0 &&
               arr[j].total_frequency < key.total_frequency) {
            arr[j + 1] = arr[j];
            j--;
        }

        arr[j + 1] = key;
    }
}

static int partition(WordData arr[], int low, int high)
{
    uint64_t pivot = arr[high].total_frequency;

    int i = low - 1;

    for (int j = low; j < high; j++) {
        if (arr[j].total_frequency > pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }

    swap(&arr[i + 1], &arr[high]);

    return i + 1;
}

void quickSort(WordData arr[], int low, int high)
{
    if (low < high) {
        int pi = partition(arr, low, high);

        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}


static void heapify(WordData arr[], int n, int i)
{
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n &&
        arr[left].total_frequency >
        arr[largest].total_frequency)
        largest = left;

    if (right < n &&
        arr[right].total_frequency >
        arr[largest].total_frequency)
        largest = right;

    if (largest != i) {
        swap(&arr[i], &arr[largest]);
        heapify(arr, n, largest);
    }
}

void heapSort(WordData arr[], int n)
{
    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(arr, n, i);

    for (int i = n - 1; i > 0; i--) {
        swap(&arr[0], &arr[i]);
        heapify(arr, i, 0);
    }

    for (int i = 0; i < n / 2; i++) {
        swap(&arr[i], &arr[n - i - 1]);
    }
}
