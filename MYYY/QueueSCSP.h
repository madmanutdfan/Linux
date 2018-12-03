#ifndef QUEUESCSP_H
#define QUEUESCSP_H

#include "Queue.h"

template<typename Elem> 
class QueueSCSP : public Queue<Elem> {
public:
	QueueSCSP();
	~QueueSCSP();

    void  enqueue(const Elem *item);
    Elem* dequeue(void);
    bool  empty(void);

private:
    volatile Element<Elem> *head;
    Element<Elem> *tail;
};
template<typename Elem> 
QueueSCSP<Elem>::QueueSCSP() {
    head = tail = (Element<Elem> *)malloc(sizeof(Element<Elem>));
    head->isTail = true;
}

template<typename Elem> 
void QueueSCSP<Elem>::enqueue(const Elem *item) {
	Element<Elem> *newElement = (Element<Elem> *)malloc(sizeof(Element<Elem>));
	newElement->isTail = true;

	tail->next = newElement;
	tail->item = item;
	tail->isTail = false;
	tail = newElement;
}

template<typename Elem> 
Elem* QueueSCSP<Elem>::dequeue() {
	Elem *item = NULL;
	volatile Element<Elem> *firstElement = NULL;

	if (!head->isTail) {
		firstElement = __sync_val_compare_and_swap(&head, head, head->next);
		item = (Elem *)firstElement->item;
	}

	delete firstElement;
	return item;
}

template<typename Elem> 
bool QueueSCSP<Elem>::empty() {
    return head->isTail;
}

template<typename Elem> 
QueueSCSP<Elem>::~QueueSCSP() {
	while (!head->isTail) {
		volatile Element<Elem> *next = head->next;
		delete head;
		head = next;
	}
	delete head;
}
#endif

