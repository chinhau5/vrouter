/*
 * heap.c
 *
 *  Created on: Oct 24, 2013
 *      Author: chinhau5
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "heap.h"

void heap_init(Heap *heap)
{
	heap->buffer = calloc(INITIAL_HEAP_SIZE, sizeof(HeapItem));
	heap->size = INITIAL_HEAP_SIZE;
	heap->tail = -1;
}

void heap_free(Heap *heap)
{
	free(heap->buffer);
	heap->buffer = NULL;
}

void heap_clear(Heap *heap)
{
	heap->tail = -1;
}

void print_heap(Heap *heap)
{
	int i;
	for (i = 0; i <= heap->tail; i++) {
		printf("%5d ", heap->buffer[i].data);
	}
}

static void heap_bubble_up(Heap *heap, int current)
{
	int parent;
	HeapItem temp;
	bool done;

	done = false;
	while (current > 0 && !done) {
		parent = (current-1)/2;
		if (heap->buffer[current].cost < heap->buffer[parent].cost) {
			temp = heap->buffer[parent];
			heap->buffer[parent] = heap->buffer[current];
			heap->buffer[current] = temp;
		} else {
			done = true;
		}
		current = parent;
	}
}

void heap_push(Heap *heap, void *data, float cost)
{
	int current;

	current = ++heap->tail;

	if (heap->tail >= heap->size) {
		heap->size += INITIAL_HEAP_SIZE;
		heap->buffer = realloc(heap->buffer, sizeof(HeapItem) * heap->size);
	}
	heap->buffer[current].cost = cost;
	heap->buffer[current].data = data;

	heap_bubble_up(heap, current);
}

void heap_update(Heap *heap, void *data, float new_cost)
{
	int i;
	int current;

	current = -1;
	for (i = 0; i < heap->tail; i++) {
		if (heap->buffer[i].data == data) {
			current = i;
			break;
		}
	}
	if (current == -1) {
		heap_push(heap, data, new_cost);
	} else {
		heap_bubble_up(heap, current);
	}
}

bool heap_pop(Heap *heap, void **data, float *cost)
{
	int parent;
	int child;
	bool empty;
	bool done;
	HeapItem temp;

	if (heap->tail < 0) {
		empty = true;
	} else {
		empty = false;

		*data = heap->buffer[0].data;
		*cost = heap->buffer[0].cost;

		heap->buffer[0] = heap->buffer[heap->tail];
		heap->tail--;
		parent = 0;
		child = 2*parent+1;
		done = false;
		while (child <= heap->tail && !done) {
			/* find the smallest child of parent */
			if (child+1 <= heap->tail && heap->buffer[child+1].cost < heap->buffer[child].cost) {
				child++;
			}

			if (heap->buffer[child].cost > heap->buffer[parent].cost) {
				done = true; /* if the smallest child of parent is larger than parent, we're done */
			} else {
				temp = heap->buffer[parent];
				heap->buffer[parent] = heap->buffer[child];
				heap->buffer[child] = temp;
				parent = child;
				child = 2*parent+1;
			}
		}
	}

	return empty;
}

bool heap_empty(Heap *heap)
{
	return heap->tail < 0;
}
