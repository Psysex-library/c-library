#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 10
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 2

typedef struct {
    int buffer[BUFFER_SIZE];
    int count;
    int in;
    int out;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} CircularBuffer;

void initBuffer(CircularBuffer *cb) {
    cb->count = 0;
    cb->in = 0;
    cb->out = 0;
    pthread_mutex_init(&cb->mutex, NULL);
    pthread_cond_init(&cb->not_empty, NULL);
    pthread_cond_init(&cb->not_full, NULL);
}

void produce(CircularBuffer *cb, int item) {
    pthread_mutex_lock(&cb->mutex);
    while (cb->count == BUFFER_SIZE) {
        pthread_cond_wait(&cb->not_full, &cb->mutex);
    }
    cb->buffer[cb->in] = item;
    cb->in = (cb->in + 1) % BUFFER_SIZE;
    cb->count++;
    pthread_cond_signal(&cb->not_empty);
    pthread_mutex_unlock(&cb->mutex);
}

int consume(CircularBuffer *cb) {
    pthread_mutex_lock(&cb->mutex);
    while (cb->count == 0) {
        pthread_cond_wait(&cb->not_empty, &cb->mutex);
    }
    int item = cb->buffer[cb->out];
    cb->out = (cb->out + 1) % BUFFER_SIZE;
    cb->count--;
    pthread_cond_signal(&cb->not_full);
    pthread_mutex_unlock(&cb->mutex);
    return item;
}

void* producer(void* arg) {
    CircularBuffer *cb = (CircularBuffer*)arg;
    for (int i = 0; i < 20; i++) {
        produce(cb, i);
        printf("Produced: %d\n", i);
        usleep(100000);
    }
    return NULL;
}

void* consumer(void* arg) {
    CircularBuffer *cb = (CircularBuffer*)arg;
    for (int i = 0; i < 20; i++) {
        int item = consume(cb);
        printf("Consumed: %d\n", item);
        usleep(150000);
    }
    return NULL;
}

int main() {
    CircularBuffer cb;
    initBuffer(&cb);
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_create(&producers[i], NULL, producer, (void*)&cb);
    }
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_create(&consumers[i], NULL, consumer, (void*)&cb);
    }
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }
    
    pthread_mutex_destroy(&cb.mutex);
    pthread_cond_destroy(&cb.not_empty);
    pthread_cond_destroy(&cb.not_full);
    return 0;
}
