#ifndef QUEUE_H
#define QUEUE_H

template<typename Elem> struct Element{
    volatile const Elem *item;
    volatile Element<Elem> *next;
    bool isTail;
};

template<typename Elem> class Queue{
public:
    virtual ~Queue() {}
    
    virtual void  enqueue(const Elem *item) = 0; 
    virtual Elem* dequeue(void) = 0;
    virtual bool  empty(void) = 0;
};

#endif
