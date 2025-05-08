#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#define RING_SIZE 5
#define NUM_ITEMS 10
typedef struct {
int data[RING_SIZE]; //init fixed size buffer
int head; //points to next read
int tail; //points to next write
int count; //curr num of elements
pthread_mutex_t lock;
pthread_cond_t not_full;
pthread_cond_t not_empty;
} RingBuffer;


void init_buffer(RingBuffer* rb){
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
    pthread_mutex_init(&rb->lock, NULL);
    pthread_cond_init(&rb->not_full, NULL);
    pthread_cond_init(&rb->not_empty, NULL);
}

bool is_empty(RingBuffer* rb){
return rb->count==0;
}

bool is_full(RingBuffer* rb){
    return rb->count==RING_SIZE;
}

void enqueue(RingBuffer* rb, int value){
    pthread_mutex_lock(&rb->lock);
    while (is_full(rb)){
        pthread_cond_wait(&rb->not_full, &rb->lock);
    }
    rb->data[rb->tail]=value;
    rb->tail = (rb->tail+1)%RING_SIZE;

    rb->count++;
    pthread_cond_signal(&rb->not_empty);
    pthread_mutex_unlock(&rb->lock);
}

void dequeue(RingBuffer* rb, int* value){
    pthread_mutex_lock(&rb->lock);
    while(is_empty(rb)){
        pthread_cond_wait(&rb->not_empty, &rb->lock);
    }
    *value=rb->data[rb->head];
    rb->head=(rb->head +1)%RING_SIZE;
    rb->count--;
    pthread_cond_signal(&rb->not_full);
    pthread_mutex_unlock(&rb->lock);
}
RingBuffer rb;

void * producer(void *arg){
    for(int i=1; i<=NUM_ITEMS;i++){
        enqueue(&rb, i);
        printf("Producer enqueued %d\n", i);
        usleep(50000);
    }
    return NULL;
}

void * consumer(void *arg){
    int val;
    for(int i=1; i<=NUM_ITEMS;i++){
        dequeue(&rb, &val);
        printf("Consumer dequeued %d\n", val);
        usleep (80000);
    }
    return NULL;
}
/*int main(){
    RingBuffer rb;
    init_buffer(&rb);
     for(int i=1; i<=6; i++){
        if (enqueue(&rb, i)){
            printf("Enqueued: %d\n", i);
        } else{
            printf("Buffer full! Could not enqueue %d\n",i);
        }
     }
     int val;
     while(dequeue(&rb, &val)){
        printf("Dequeued: %d\n", val);
     }
     return 0;

} */
int main(){
    pthread_t prod_thread, cons_thread;
    init_buffer(&rb);

    pthread_create(&prod_thread, NULL, producer, NULL);
    pthread_create(&cons_thread, NULL, consumer, NULL);

    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    pthread_mutex_destroy(&rb.lock);
    pthread_cond_destroy(&rb.not_full);
    pthread_cond_destroy(&rb.not_empty);

    return 0;
}