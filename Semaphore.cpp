//
// Created by student on 12/29/20.
//

#include "Semaphore.hpp"


Semaphore::Semaphore(){
    this->value=0;
    pthread_mutex_init(&this->mutex,NULL);
    pthread_cond_init(&this->cond,NULL);

}

Semaphore::Semaphore(unsigned val){
    this->value=val;
    pthread_mutex_init(&this->mutex,NULL);
    pthread_cond_init(&this->cond,NULL);

}

void Semaphore::up() {
    pthread_mutex_lock(&this->mutex);
    this->value++;
    pthread_cond_signal(&this->cond);
    pthread_mutex_unlock(&this->mutex);
}


void Semaphore::down(){
    pthread_mutex_lock(&this->mutex);
    while(this->value==0){
        pthread_cond_wait(&this->cond,&this->mutex);
    }
    this->value--;
    pthread_mutex_unlock(&this->mutex);
}