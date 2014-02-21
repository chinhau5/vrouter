/*
 * heap.h
 *
 *  Created on: Oct 24, 2013
 *      Author: chinhau5
 */

#ifndef HEAP_H_
#define HEAP_H_

typedef struct _HeapItem {
	void *data;
	float cost;
} HeapItem;

typedef struct _Heap {
	struct _HeapItem *buffer;
	int size;
	int tail;
} Heap;

#define INITIAL_HEAP_SIZE 200

void heap_init(Heap *heap);
void heap_free(Heap *heap);
void heap_push(Heap *heap, void *data, float cost);
void heap_update(Heap *heap, void *data, float new_cost);
bool heap_pop(Heap *heap, void **data, float *cost);
bool heap_empty(Heap *heap);
void print_heap(Heap *heap);
void heap_clear(Heap *heap);

#endif /* HEAP_H_ */
