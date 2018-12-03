#include <cstdlib>
#include <stdio.h>
#include <pthread.h>

#include "QueueMCSP.h"
#include "QueueMCMP.h"
#include "Queue.h"

using namespace std;

#define N 50
#define readCnt 5

pthread_t writers[readCnt];
pthread_t readers[readCnt];

QueueMCSP<int> *MCSPqueue;
volatile int arr[N];
volatile int arr_test[N];

volatile int test2Count = 0;
volatile int test2WorkingCount = 0;

volatile int time_test = 0;

void *testMCSP_1_proc(void *x)
{
   while (!MCSPqueue->empty()) {
        int *res = MCSPqueue->dequeue();
        if (res != NULL){
            arr_test[*res]++;
        }
   }
   return NULL;
}

bool testMCSP_1() {
    for (int i = 0; i < N; ++i)
        MCSPqueue->enqueue((int *)arr + i);
    for (int i = 0; i < readCnt; ++i)
        pthread_create(readers + i, NULL, testMCSP_1_proc, NULL);
    for (int i = 0; i < readCnt; ++i)
        pthread_join(readers[i], NULL);
    
    while (!MCSPqueue->empty());
    
    for (int i = 0; i < N; ++i)
        if (arr_test[i] != 1)
            return false;
        
    return true;
}

template<typename Elem>
void *test1Write_NRead_proc_read(void* x)
{
   Queue<Elem> *q = (Queue<Elem> *)x;
   while (test2WorkingCount != 1 || !q->empty()) {
        //printf("count  = %i\n", test2CountN);
        if (q->empty()) continue;
        int *res = q->dequeue();
        if (res != NULL){
            arr_test[*res]++;
        }    
        //printf("1\n");
        arr_test;
   }
   test2Count++;
   return NULL;
}

template<typename Elem>
void *testNWrite_1Read_proc_read(void* x)
{
   Queue<Elem> *q = (Queue<Elem> *)x;
   while (test2WorkingCount < readCnt-1 || !q->empty()) {
        //printf("count  = %i\n", test2CountN);
        if (q->empty()) continue;
        int *res = q->dequeue();
        if (res != NULL){
            arr_test[*res]++;
        }    
        //printf("1\n");
        arr_test;
   }
   test2Count++;
   return NULL;
}

template<typename Elem>
void *testNWrite_1Read_proc_write(void* x)
{
    Queue<Elem> *q = (Queue<Elem> *)x;
    for (int i = 0; i < N; ++i){
        q->enqueue((int *)arr + i);
    }
    test2WorkingCount++;
    return NULL;
}

template<typename Elem>
bool testNWrite_1Read(Queue<Elem> *q) {
    test2WorkingCount = 0;
    test2Count = 0;
    
    for (int i = 0; i < N; ++i) {
        arr_test[i] = 0;
    }
    for (int i = 0; i < readCnt; ++i)
        pthread_create(writers + i, NULL, testNWrite_1Read_proc_write<Elem>, q);
    
    pthread_create(readers, NULL, testNWrite_1Read_proc_read<Elem>, q);
    for (int i = 0; i < readCnt; ++i)
        pthread_join(writers[i], NULL);//pthread_detach(readers[i]);
    pthread_join(readers[0], NULL);
    
   // for (int i = 0; test2Count < readCnt-1; i++);
    
    for (int i = 0; i < N; ++i)
        if (arr_test[i] > readCnt){
            printf("arr arr_test[%i] = %i\n", i, arr_test[i]);
            return false;
        }
        
    return true;
}

template<typename Elem>
bool test1Write_NRead(Queue<Elem> *q) {
    test2WorkingCount = 0;
    test2Count = 0;
    
    for (int i = 0; i < N; ++i) {
        arr_test[i] = 0;
    }
    for (int i = 0; i < readCnt; ++i)
        pthread_create(readers + i, NULL, test1Write_NRead_proc_read<Elem>, q);
    
    pthread_create(writers, NULL, testNWrite_1Read_proc_write<Elem>, q);
    for (int i = 0; i < readCnt; ++i)
        pthread_join(readers[i], NULL);//pthread_detach(readers[i]);
    pthread_join(writers[0], NULL);
    
    //for (int i = 0; test2Count != readCnt; i++);
    
    for (int i = 0; i < N; ++i)
        if (arr_test[i] != 1){
            printf("arr arr_test[%i] = %i\n", i, arr_test[i]);
            return false;
        }
        
    return true;
}

template<typename Elem>
bool testNWrite_NRead(Queue<Elem> *q) {
    test2WorkingCount = 0;
    test2Count = 0;
    
    for (int i = 0; i < N; ++i) {
        arr_test[i] = 0;
    }
    for (int i = 0; i < readCnt; ++i)
        pthread_create(readers + i, NULL, testNWrite_1Read_proc_read<Elem>, q);
    for (int i = 0; i < readCnt; ++i)
        pthread_create(writers + i, NULL, testNWrite_1Read_proc_write<Elem>, q);
    for (int i = 0; i < readCnt; ++i)
        pthread_join(readers[i], NULL);//pthread_detach(readers[i]);
    for (int i = 0; i < readCnt; ++i)
        pthread_join(writers[i], NULL);
    
    
    for (int i = 0; i < N; ++i)
        if (arr_test[i] > readCnt){
            printf("arr arr_test[%i] = %i\n", i, arr_test[i]);
            return false;
        }
        
    return true;
}








template<typename Elem>
void *timeTest_proc_write(void* x)
{
    Queue<Elem> *q = (Queue<Elem> *)x;
    for (int i = 0; i < N; ++i) {
        q->enqueue((int *)arr + i);
    }
    return NULL;
}





template<typename Elem>
void *testNWrite_1Read_proc_write_time(void* x)
{
    Queue<Elem> *q = (Queue<Elem> *)x;
    for (int i = 0; i < N; ++i){
        unsigned int start_time =  clock();
        MCSPqueue->enqueue((int *)arr + i);
        time_test += clock() - start_time;
    }
    test2WorkingCount++;
    return NULL;
}

template<typename Elem>
void *test1Write_NRead_proc_read_time(void* x)
{
   Queue<Elem> *q = (Queue<Elem> *)x;
   while (test2WorkingCount != 1 || !MCSPqueue->empty()) {
        //printf("count  = %i\n", test2CountN);
        if (MCSPqueue->empty()) continue;
        unsigned int start_time =  clock();
        int *res = MCSPqueue->dequeue();
        
        if (res != NULL){
            time_test += clock() - start_time;
            arr_test[*res]++;
        }    
        //printf("1\n");
        arr_test;
   }
   test2Count++;
   return NULL;
}

template<typename Elem>
int test1Write_NRead_time(Queue<Elem> *q) {
    test2WorkingCount = 0;
    test2Count = 0;
    time_test = 0;
    
    for (int i = 0; i < N; ++i) {
        arr_test[i] = 0;
    }
    for (int i = 0; i < readCnt; ++i)
        pthread_create(readers + i, NULL, test1Write_NRead_proc_read_time<Elem>, q);
    
    pthread_create(writers, NULL, testNWrite_1Read_proc_write_time<Elem>, q);
    for (int i = 0; i < readCnt; ++i)
        pthread_join(readers[i], NULL);//pthread_detach(readers[i]);
    pthread_join(writers[0], NULL);
    
    //for (int i = 0; test2Count != readCnt; i++);
   
        
    return time_test;
}

int main(int argc, char** argv) {
    MCSPqueue = new QueueMCSP<int>;

    for (int i = 0; i < N; ++i)
	{
        arr[i] = i;
    }

    for (int k = 0; k < 1000; ++k)
	{
        for (int i = 0; i < N; ++i)
		{
            arr_test[i] = 0;
            arr[i] = i;
        }
        if (!testMCSP_1()) 
		{
            printf("Test 1 err\n");
            return 0;
        }
    }
    printf("Test 1 worked 1000 times\n");

    for (int k = 0; k < 100; ++k) {
        QueueMCSP<int> MCSPq;
        if (!test1Write_NRead<int>(&MCSPq)) {
            printf("Test 2 err\n");
            return 0;
        }
    }
    printf("Test 2 worked 100 times\n");
    
    for (int k = 0; k < 100; ++k) {
        QueueMCMP<int> MCMPq;
        if (!test1Write_NRead<int>(&MCMPq)) {
            printf("Test test1Write_NRead for MCMP err\n");
            return 0;
        }
    }
    printf("Test test1Write_NRead for MCMP worked 100 times\n");
    
    for (int k = 0; k < 100; ++k) {
        QueueMCMP<int> MCMPq;
        if (!testNWrite_1Read<int>(&MCMPq)) {
            printf("Test testNWrite_1Read for MCMP err\n");
            return 0;
        }
    }
    printf("Test testNWrite_1Read for MCMP worked 100 times\n");
    
    for (int k = 0; k < 100; ++k) {
        QueueMCMP<int> MCMPq;
        if (!testNWrite_NRead<int>(&MCMPq)) {
            printf("Test testNWrite_NRead for MCMP err\n");
            return 0;
        }
    }
    printf("Test testNWrite_NRead for MCMP worked 100 times\n");
    
    int timeT = 0;
    QueueMCSP<int> MCSPq;
    QueueMCMP<int> MCMPq;

    timeT = 0;
    for (int k = 0; k < 500; ++k) 
	{
        timeT += test1Write_NRead_time<int>(&MCSPq);
    }
    printf("Time for MCSP = %i\n", timeT/500);
    
    timeT = 0;
    for (int k = 0; k < 500; ++k)
	{
        timeT += test1Write_NRead_time<int>(&MCMPq);
    }
    printf("Time for MCMP = %i\n", timeT/500);
    
    
    return 0;
}

