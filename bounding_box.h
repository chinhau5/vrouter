/*
 * bounding_box.h
 *
 *  Created on: Nov 15, 2013
 *      Author: chinhau5
 */

#ifndef BOUNDING_BOX_H_
#define BOUNDING_BOX_H_

#include <stdbool.h>

typedef struct _s_bounding_box {
	int left, right;
	int top, bottom;
	int area;
} s_bounding_box;

bool aabb_intersect(s_bounding_box *a, s_bounding_box *b);

#endif /* BOUNDING_BOX_H_ */
