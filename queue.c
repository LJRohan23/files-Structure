/*****************************************************************************
 * Copyright (C) Rohan Yogi. yogir15.comp@coep.ac.in
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#include "queue.h"
int qfull(queue *q) {
	if((q->front) == 0 && (q->rear) == QMAX-1 || (q->front) - 1 == (q->rear))
		return 1;
	return 0;
}
int qempty(queue *q) {
	return (q->front == -1 );
}
void qinit(queue *q) {
	q->front = q->rear = -1;
}
void enqueue(queue *q, char ch) {
	if(q->rear== -1) {
		q->front = q->rear = 0;
	}
	else
		if(q->rear == QMAX - 1) {
			q->rear = 0;
		}
		else
			q->rear ++;
	q->arr[q->rear] = ch;
}
char dequeue(queue *q) {
	char temp = q->arr[q->front];
	if(q->front == q->rear)
		q->front = q->rear = -1;
	else
		if(q->front == QMAX - 1)
			q->front = 0;
		else
				q->front++;
	return temp;
}

