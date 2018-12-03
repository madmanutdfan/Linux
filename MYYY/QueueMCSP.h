#ifndef QUEUEMCSP_H
#define QUEUEMCSP_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "Queue.h"

template<typename Elem> 
class QueueMCSP : public Queue<Elem>{
public:
    QueueMCSP();
	~QueueMCSP();

    void  enqueue(const Elem *item);
    Elem* dequeue(void);
    bool  empty(void);

private:
    volatile Element<Elem> *head;
    Element<Elem> *tail;
    
    volatile long long currentNum;
    volatile long long maxNum;
    volatile int lock;
};

template<typename Elem>
QueueMCSP<Elem>::QueueMCSP() {
    head = tail = (Element<Elem> *)malloc(sizeof(Element<Elem>));
    head->isTail = true;
    currentNum = maxNum = 0;
    lock = 0;
}

template<typename Elem>
void QueueMCSP<Elem>::enqueue(const Elem *item) {
	Element<Elem> *newElement = (Element<Elem> *)malloc(sizeof(Element<Elem>));
	newElement->isTail = true;

	tail->next = newElement;
	tail->item = item;
	tail->isTail = false;
	tail = newElement;
}

template<typename Elem>
Elem* QueueMCSP<Elem>::dequeue() {
    Elem *item = NULL;
    volatile Element<Elem> *firstElement = NULL;
    
    long long num = __sync_fetch_and_add(&maxNum, 1);

	while (currentNum < num);

    while (__sync_lock_test_and_set(&lock, 1))
		while (lock)
			pthread_yield();

    if (!head->isTail) {
		firstElement = __sync_val_compare_and_swap(&head, head, head->next); 
        item = (Elem *)firstElement->item;
    }

    __sync_lock_release(&lock);
    currentNum++;
    
    delete firstElement;
    return item;
}

template<typename Elem>
bool QueueMCSP<Elem>::empty() {
	return head->isTail;
}

template<typename Elem>
QueueMCSP<Elem>::~QueueMCSP() {
	while (!head->isTail) {
		volatile Element<Elem> *next = head->next;
		delete head;
		head = next;
	}
	delete head;
}
#endif /* QUEUEMCSP_H */
