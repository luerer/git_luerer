#ifndef NET_MMT_MMTP_RINGQ_H_
#define NET_MMT_MMTP_RINGQ_H_

#ifdef __cplusplus
extern "C" {
#endif 

#define QUEUE_MAX 1500
/*

http://blog.csdn.net/sking002007/article/details/6584590

*/
typedef struct ringq{
   int head; /* 头部，出队列方向*/
   int tail; /* 尾部，入队列方向*/ 
   int tag ; /* 为空还是为满的标志位*/
    int size ; /* 队列总尺寸 */
   int space[QUEUE_MAX]; /* 队列空间 */
}RINGQ;

/* 
  第一种设计方法:
     当head == tail 时，tag = 0 为空，等于 = 1 为满。
*/

extern int ringq_init(RINGQ * p_queue);

extern int ringq_free(RINGQ * p_queue);


/* 加入数据到队列 */
extern int ringq_push(RINGQ * p_queue,int data);

/* 从队列取数据 */
extern int ringq_poll(RINGQ * p_queue,int *p_data);


#define ringq_is_empty(q) ( (q->head == q->tail) && (q->tag == 0))

#define ringq_is_full(q) ( (q->head == q->tail) && (q->tag == 1))

#define print_ringq(q) printf("ring head %d,tail %d,tag %d\n", q->head,q->tail,q->tag);
#ifdef __cplusplus
}
#endif 


//http://stackoverflow.com/questions/827691/how-do-you-implement-a-circular-buffer-in-c/1771607#1771607
typedef struct circular_buffer
{
    void *buffer;     // data buffer
    void *buffer_end; // end of data buffer
    size_t capacity;  // maximum number of items in the buffer
    size_t count;     // number of items in the buffer
    size_t sz;        // size of each item in the buffer
    void *head;       // pointer to head
    void *tail;       // pointer to tail
} circular_buffer;
void cb_init(circular_buffer *cb, size_t capacity, size_t sz);
void cb_free(circular_buffer *cb);
void cb_push_back(circular_buffer *cb, const void *item);
void cb_pop_front(circular_buffer *cb, void *item);

#endif
