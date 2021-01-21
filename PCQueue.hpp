#ifndef _QUEUEL_H
#define _QUEUEL_H
#include "Headers.hpp"


// Single Producer - Multiple Consumer queue
template <typename T>class PCQueue
{

public:

    PCQueue();  // constructor.
    // Blocks while queue is empty. When queue holds items, allows for a single
    // thread to enter and remove an item from the front of the queue and return it.
    // Assumes multiple consumers.
    T pop();

    // Allows for producer to enter with *minimal delay* and push items to back of the queue.
    // Hint for *minimal delay* - Allow the consumers to delay the producer as little as possible.
    // Assumes single producer
    void push(const T& item);

private:
    queue <T> items;
    int producer_inside, producer_waiting, consumer_inside;
    pthread_mutex_t mutex;
    pthread_cond_t cond_consumers,cond_producer;
    void consumers_lock() ;
    void consumers_unlock();
    void producer_lock();
    void producer_unlock();



};
template <typename T>
PCQueue< T>::PCQueue() {
    consumer_inside = 0;
    producer_waiting = 0;
    producer_inside = 0;
    pthread_cond_init(&cond_consumers, NULL);
    pthread_cond_init(&cond_producer, NULL);
    pthread_mutex_init(&mutex, NULL);
}


template <typename T>
T PCQueue<T>::pop() {
    T del;
    consumers_lock();
    del=items.front();
    items.pop();
    consumers_unlock();
    return del;
}


template <typename T>
void PCQueue<T>::push(const T &item) {
    producer_lock();
    items.push(item);
    producer_unlock();
}


template <typename T>
void PCQueue<T>::consumers_lock() {
    pthread_mutex_lock(&mutex);
    while (producer_inside >0 ||consumer_inside >0 || producer_waiting > 0 || items.size()==0)
    {
        pthread_cond_wait(&cond_consumers, &mutex);
    }
    consumer_inside++;
    pthread_mutex_unlock(&mutex);
}


template <typename T>
void PCQueue<T>::consumers_unlock() {
    pthread_mutex_lock(&mutex);
    consumer_inside--;
    pthread_cond_broadcast(&cond_consumers);
    if (consumer_inside == 0)
        pthread_cond_signal(&cond_producer);
    pthread_mutex_unlock(&mutex);
}


template <typename T>
void PCQueue<T>::producer_lock() {
    pthread_mutex_lock(&mutex);
    producer_waiting++;
    while (consumer_inside > 0 || producer_inside > 0) {
        pthread_cond_wait(&cond_producer, &mutex);
    }
    producer_waiting--;
    producer_inside++;
    pthread_mutex_unlock(&mutex);
}


template <typename T>
void PCQueue<T>::producer_unlock() {
    pthread_mutex_lock(&mutex);
    producer_inside--;
    pthread_cond_broadcast(&cond_consumers);
    pthread_cond_signal(&cond_producer);
    pthread_mutex_unlock(&mutex);
}
#endif
