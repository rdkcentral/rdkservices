#ifndef BUFFERQUEUE_H_
#define BUFFERQUEUE_H_

#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <cstring>
#include <queue>
#include <stdio.h>
#include <unistd.h>
#include "logger.h"

struct Buffer
{
    Buffer()
    {
    }
    void fillBuffer(const void *ptr,int len);
    int getLength();
    char *getBuffer();   
    void deleteBuffer();
    char *buff;
    int length;
};

class BufferQueue
{
    public:
    void add(Buffer* item);
    Buffer* remove();
    BufferQueue(int);
    bool isEmpty();
    bool isFull();
    void clear();
    void preDelete();
    int count();
    ~BufferQueue();

    private:
    std::queue<Buffer*> m_buffer;
    pthread_mutex_t m_mutex;
    sem_t m_sem_full;
    sem_t m_sem_empty;
};
#endif
