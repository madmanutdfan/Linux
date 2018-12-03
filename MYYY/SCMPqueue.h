#ifndef SCMPQUEUE_H
#define SCMPQUEUE_H
#include"queue.h"
#include <cstddef>
#include "utils.h"
#include<stdio.h>
#include<unistd.h>

template <typename Elem>

class SCMPqueue : public Queue <Elem> {
public:
  SCMPqueue(int numThreads) ;
  ~SCMPqueue();
  Node<Elem>* head, * tail;
  OpDesc<Elem> ** state;
  int size;
  int numThreads;
  void enqueue(const Elem item, int tid); 
  Elem dequeue(int tid); 
  bool empty(void);
  long maxPhase();
  void help(long phase);
  bool isStillPending(int tid, long ph);
  void help_enqueue(int tid, long phase);
  void help_finish_enqueue();

  void help_dequeue(int tid, long phase);
  void help_finish_dequeue(); 
};



template <typename Elem>
SCMPqueue<Elem>::SCMPqueue (int numThreads) {
  Node<Elem> * sentinel = new Node<Elem>(-1, -1);
  head = sentinel;
  tail = sentinel;
  size = 0;
  this->numThreads = numThreads;
  state = new OpDesc<Elem> *[numThreads];
  for (int i = 0; i < numThreads; i++) {
    state[i] = new OpDesc<Elem>(-1, false, true, NULL); 
  }
}

template <typename Elem>
SCMPqueue<Elem>::~SCMPqueue () {
  for (int i = 0; i < numThreads; i++) {
    delete state[i]; 
  }
  delete state;
}

template <typename Elem>
void SCMPqueue<Elem>::help(long phase) {
  for (int i = 0; i < numThreads; i++) {
    OpDesc<Elem> * desc = state[i];
    if (desc->pending && desc->phase <= phase) {
      if (desc->enqueue) {
        help_enqueue(i, phase);
      } else {
        help_dequeue(i, phase);
      }
    }
  }
}

template <typename Elem>
long SCMPqueue<Elem>::maxPhase() {
  long maxPhase = -1;
  for (int i = 0; i < numThreads; i++) {
    long phase = state[i]->phase;
    if (phase > maxPhase) {
      maxPhase = phase;
    }
  }
  return maxPhase;
}

template <typename Elem>
bool SCMPqueue<Elem>::isStillPending(int tid, long ph) {
  return state[tid]->pending && state[tid]->phase <= ph;
}

//////////////////////// ENQUEUE OPERATIONS //////////////////////////////////

template <typename Elem>
void SCMPqueue<Elem>::enqueue(Elem value, int tid) {
  long phase = maxPhase() + 1;
  state[tid] = new OpDesc<Elem>(phase, true, true, new Node<Elem>(value, tid));
  help(phase);
  help_finish_enqueue();
  __sync_bool_compare_and_swap(&(size), size, size + 1);
}

template <typename Elem>
void SCMPqueue<Elem>::help_enqueue(int tid, long phase) {
  while (isStillPending(tid, phase)) {
    Node<Elem> * last = tail;
    Node<Elem> * next = last->next;
    if (last == tail) {
      if (next == NULL) {
        if (isStillPending(tid, phase)) {
          if (__sync_bool_compare_and_swap(&(last->next), next, state[tid]->node))   {
            help_finish_enqueue();
            return;
          }
        }			
      } else {
        help_finish_enqueue();
      }
    }
  }
}

template <typename Elem>
void SCMPqueue<Elem>::help_finish_enqueue() {
  Node<Elem> * last = tail;
  Node<Elem> * next = last->next;
  if (next != NULL) {
    int tid = next->enqTid;
    OpDesc<Elem> * curDesc = state[tid];
    if (last == tail && state[tid]->node == next) {
      OpDesc<Elem> * newDesc = new OpDesc<Elem>(state[tid]->phase, false, true, next);
      __sync_bool_compare_and_swap(&(state[tid]), curDesc, newDesc);
      __sync_bool_compare_and_swap(&(tail), last, next);
    }
  }
}


///////////////////// DEQUEUE OPERATION ////////////////////////////

template <typename Elem>
Elem SCMPqueue<Elem>::dequeue(int tid) {
  long phase = maxPhase() + 1;
  __sync_bool_compare_and_swap(&(size), size, size - 1);
  state[tid] = new OpDesc<Elem>(phase, true, false, NULL);
  help(phase);
  help_finish_dequeue();
  Node<Elem> * node = state[tid]->node;
  if (node == NULL) {
    printf("EMPTY EXCEPTION\n");
    return -1;
  }
  return (node->next)->value;
}

template <typename Elem>
void SCMPqueue<Elem>::help_dequeue(int tid, long phase) {
  while (isStillPending(tid, phase)) {
    Node<Elem> * first = head;
    Node<Elem> * last = tail;
    Node<Elem> * next = first->next;
    if (first == head) {
      if (first == last) {
        if (next == NULL) {
          OpDesc<Elem> * curDesc = state[tid];
          if (last == tail && isStillPending(tid, phase)) {
            OpDesc<Elem> * newDesc = new OpDesc<Elem>(state[tid]->phase, false, false, NULL);
            //state.compareAndSet(tid, curDesc, newDesc);
            __sync_bool_compare_and_swap(&(state[tid]), curDesc, newDesc);
          }
        } else {
          help_finish_enqueue();
        }
      } else {
        OpDesc<Elem> * curDesc = state[tid];
        Node<Elem> * node = curDesc->node;
        if (!isStillPending(tid, phase)) break;
        if (first == head && node != first) {
          OpDesc<Elem> * newDesc = new OpDesc<Elem>(state[tid]->phase, true, false, first);
          if(__sync_bool_compare_and_swap(&(state[tid]), curDesc, newDesc)) {
            continue;
          }
        }
        __sync_bool_compare_and_swap(&(first->deqTid), -1, tid);
        help_finish_dequeue();
      }
    }
  }
}

template <typename Elem>
void SCMPqueue<Elem>::help_finish_dequeue() {
  Node<Elem> * first = head;
  Node<Elem> * next = first->next;
  int tid = first->deqTid;
  if (tid != -1) {
    OpDesc<Elem> * curDesc = state[tid];
    if (first == head && next != NULL) {
      OpDesc<Elem> * newDesc = new OpDesc<Elem>(state[tid]->phase, false, false, state[tid]->node);
      __sync_bool_compare_and_swap(&(state[tid]), curDesc, newDesc);
      __sync_bool_compare_and_swap(&(head), first, next);
    }
  }
}

template <typename Elem>
bool SCMPqueue<Elem>::empty(void) {
  if(size == 0) return true;
  return false;
} 

#endif

