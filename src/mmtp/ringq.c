#include <stdio.h>
#include "ringq.h"
#include <stdlib.h>
#include <string.h>

int ringq_init(RINGQ * p_queue)
{
   p_queue->size = QUEUE_MAX ;
   
   p_queue->head = 0;
   p_queue->tail = 0;
   
   p_queue->tag = 0;
   
   return 0;
}

int ringq_free(RINGQ * p_queue)
{
  return 0;
}


int ringq_push(RINGQ * p_queue,int data)
{
//  print_ringq(p_queue);
  
  if(ringq_is_full(p_queue))
   {
     
     printf("ringq is full\n");
     return -1;
   }
      
   p_queue->space[p_queue->tail] = data;
   
   p_queue->tail = (p_queue->tail + 1) % p_queue->size ;
   
   /* 这个时候一定队列满了*/
   if(p_queue->tail == p_queue->head)
    {
       p_queue->tag = 1;
    }

    return p_queue->tag ;  
}

int ringq_poll(RINGQ * p_queue,int * p_data)
{
//   print_ringq(p_queue);
  if(ringq_is_empty(p_queue))
   {
      
      printf("ringq is empty\n");
     return -1;
   }
   
   *p_data = p_queue->space[p_queue->head];
   
   p_queue->head = (p_queue->head + 1) % p_queue->size ;
   
    /* 这个时候一定队列空了*/
   if(p_queue->tail == p_queue->head)
    {
       p_queue->tag = 0;
    }    
    return p_queue->tag ;
}

//http://stackoverflow.com/questions/827691/how-do-you-implement-a-circular-buffer-in-c/1771607#1771607


void cb_init(circular_buffer *cb, size_t capacity, size_t sz)
{
    cb->buffer = malloc(capacity * sz);
    if(cb->buffer == NULL)
        // handle error
    cb->buffer_end = (char *)cb->buffer + capacity * sz;
    cb->capacity = capacity;
    cb->count = 0;
    cb->sz = sz;
    cb->head = cb->buffer;
    cb->tail = cb->buffer;
}

void cb_free(circular_buffer *cb)
{
    free(cb->buffer);
    // clear out other fields too, just to be safe
}

void cb_push_back(circular_buffer *cb, const void *item)
{
    if(cb->count == cb->capacity)
        // handle error
    memcpy(cb->head, item, cb->sz);
    cb->head = (char*)cb->head + cb->sz;
    if(cb->head == cb->buffer_end)
        cb->head = cb->buffer;
    cb->count++;
}

void cb_pop_front(circular_buffer *cb, void *item)
{
    if(cb->count == 0)
        // handle error
    memcpy(item, cb->tail, cb->sz);
    cb->tail = (char*)cb->tail + cb->sz;
    if(cb->tail == cb->buffer_end)
        cb->tail = cb->buffer;
    cb->count--;
}
