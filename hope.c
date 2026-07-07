#include <stdio.h>

#define MAX_SIZE 100

// Structure to represent the Heap
struct MaxHeap {
    int data[MAX_SIZE];
    int size;
};

// Helper function to swap two integers
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Function to insert an element into the heap
void insert(struct MaxHeap *heap, int value) {
    if (heap->size >= MAX_SIZE) {
        printf("Heap overflow! Cannot insert %d\n", value);
        return;
    }

    // Insert the new element at the end of the array
    heap->data[heap->size] = value;
    int current = heap->size;
    heap->size++;

    // Heapify-Up (Bubble up): Move the element up until max-heap property is satisfied
    while (current > 0) {
        int parent = (current - 1) / 2;
        
        // If child is greater than parent, swap them
        if (heap->data[current] > heap->data[parent]) {
            swap(&heap->data[current], &heap->data[parent]);
            current = parent; // Move up to parent's index
        } else {
            break;
        }
    }
}

// Function to fix the heap down from a given index (Heapify-Down)
void heapifyDown(struct MaxHeap *heap, int index) {
    int largest = index;
    int leftChild = 2 * index + 1;
    int rightChild = 2 * index + 2;

    // Check if left child is larger than current largest
    if (leftChild < heap->size && heap->data[leftChild] > heap->data[largest]) {
        largest = leftChild;
    }

    // Check if right child is larger than current largest
    if (rightChild < heap->size && heap->data[rightChild] > heap->data[largest]) {
        largest = rightChild;
    }

    // If largest is not the root, swap and continue heapifying down
    if (largest != index) {
        swap(&heap->data[index], &heap->data[largest]);
        heapifyDown(heap, largest);
    }
}

// Function to extract (remove and return) the maximum element (root)
int extractMax(struct MaxHeap *heap) {
    if (heap->size <= 0) {
        printf("Heap underflow! Heap is empty.\n");
        return -1;
    }

    // The root contains the maximum element
    int maxVal = heap->data[0];

    // Replace the root with the last element in the array
    heap->data[0] = heap->data[heap->size - 1];
    heap->size--;

    // Rebalance the tree by shifting the new root downwards
    heapifyDown(heap, 0);

    return maxVal;
}

// Function to print the array representation of the heap
void printHeap(struct MaxHeap *heap) {
    for (int i = 0; i < heap->size; i++) {
        printf("%d ", heap->data[i]);
    }
    printf("\n");
}

int main() {
    struct MaxHeap heap;
    heap.size = 0; // Initialize empty heap

    // Insert elements
    insert(&heap, 40);
    insert(&heap, 20);
    insert(&heap, 30);
    insert(&heap, 10);
    insert(&heap, 50);

    printf("Max-Heap array structure: ");
    printHeap(&heap);

    // Extract maximum value
    printf("Extracted Max: %d\n", extractMax(&heap));

    printf("Heap array after extraction: ");
    printHeap(&heap);

    return 0;
}

