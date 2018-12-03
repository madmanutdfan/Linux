#include <cstdlib>
#include <stdio.h>
#include <pthread.h>

#include "QueueMCSP.h"
#include "QueueSCSP.h"
#include "Queue.h"

using namespace std;

#define N 50
#define rwCnt 5
#define CCnt1 10
#define CCnt2 100
#define CCnt3 1000
#define CCnt4 10000

pthread_t writers[1];
pthread_t readers[rwCnt];

QueueMCSP<int> *MCSPqueue;
volatile int arr[N];
volatile int arr_test[N];

volatile int testWorkingThreads = 0;

volatile int time_test = 0;

void *testMCSP_1_func(void* x){
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
    for (int i = 0; i < rwCnt; ++i)
        pthread_create(readers + i, NULL, testMCSP_1_func, NULL);
    for (int i = 0; i < rwCnt; ++i)
        pthread_join(readers[i], NULL);
    
    while (!MCSPqueue->empty());
    
    for (int i = 0; i < N; ++i)
        if (arr_test[i] != 1)
            return false;
        
    return true;
}

template<typename Elem>
void *testWriter_Readers_func_read(void* x){
   Queue<Elem> *q = (Queue<Elem> *)x;
   while (testWorkingThreads != 1 || !q->empty()) {
        if (q->empty()) continue;
        int *res = q->dequeue();
        if (res != NULL){
            arr_test[*res]++;
        }    
        arr_test;
   }
   return NULL;
}

template<typename Elem>
void *testWriter_Reader_func_read(void* x){
   Queue<Elem> *q = (Queue<Elem> *)x;
   while (testWorkingThreads < rwCnt - 1 || !q->empty()) {
	   if (q->empty()) continue;
        int *res = q->dequeue();
        if (res != NULL) arr_test[*res]++;
        arr_test;
   }
   return NULL;
}

template<typename Elem>
void *testWriter_Reader_func_write(void* x){
    Queue<Elem> *q = (Queue<Elem> *)x;
    for (int i = 0; i < N; ++i){
        q->enqueue((int *)arr + i);
    }
    testWorkingThreads++;
    return NULL;
}

template<typename Elem>
bool testWriter_Readers(Queue<Elem> *q) {
    testWorkingThreads = 0;
    
    for (int i = 0; i < N; ++i) {
        arr_test[i] = 0;
    }
    for (int i = 0; i < rwCnt; ++i)
        pthread_create(readers + i, NULL, testWriter_Readers_func_read<Elem>, q);
    
    pthread_create(writers, NULL, testWriter_Reader_func_write<Elem>, q);
    for (int i = 0; i < rwCnt; ++i)
        pthread_join(readers[i], NULL);
    pthread_join(writers[0], NULL);
    
    for (int i = 0; i < N; ++i)
        if (arr_test[i] != 1){
            printf("arr arr_test[%i] = %i\n", i, arr_test[i]);
            return false;
        }
        
    return true;
}

template<typename Elem>
bool testWriter_Reader(Queue<Elem> *q) {
    testWorkingThreads = 0;
    
    for (int i = 0; i < N; ++i) {
        arr_test[i] = 0;
    }

	pthread_create(writers, NULL, testWriter_Reader_func_write<Elem>, q);
    pthread_create(readers, NULL, testWriter_Reader_func_read<Elem>, q);

    pthread_join(readers[0], NULL);
	pthread_join(writers[0], NULL);
    
    for (int i = 0; i < N; ++i)
        if (arr_test[i] > rwCnt){
            printf("arr arr_test[%i] = %i\n", i, arr_test[i]);
            return false;
        }
        
    return true;
}
//
// TIME
//
template<typename Elem>
void *test_time_Writer_Reader_func_write(void* x)
{
    Queue<Elem> *q = (Queue<Elem> *)x;
    for (int i = 0; i < N; ++i){
        unsigned int start_time = (int)clock();
        MCSPqueue->enqueue((int *)arr + i);
        time_test += (int)clock() - start_time;
    }
    testWorkingThreads++;
    return NULL;
}

template<typename Elem>
void *test_time_Writer_Reader_func_read(void* x)
{
   Queue<Elem> *q = (Queue<Elem> *)x;
   while (testWorkingThreads != 1 || !MCSPqueue->empty()) {
        if (MCSPqueue->empty()) continue;
        unsigned int start_time =  (int)clock();
        int *res = MCSPqueue->dequeue();
        
        if (res != NULL){
            time_test += (int)clock() - start_time;
            arr_test[*res]++;
        }    
        arr_test;
   }
   return NULL;
}

template<typename Elem>
int test_Time_Writer_Reader(Queue<Elem> *q) {
    testWorkingThreads = 0;
    time_test = 0;
    
    for (int i = 0; i < N; ++i) {
        arr_test[i] = 0;
    }
    pthread_create(readers, NULL, test_time_Writer_Reader_func_read<Elem>, q);
    pthread_create(writers, NULL, test_time_Writer_Reader_func_write<Elem>, q);

    pthread_join(readers[0], NULL);
    pthread_join(writers[0], NULL);
   
    return time_test;
}

int main(int argc, char** argv) {
    MCSPqueue = new QueueMCSP<int>;

    for (int i = 0; i < N; ++i) arr[i] = i;

	for (int k = 0; k < CCnt1; ++k) {
		for (int i = 0; i < N; ++i) {
			arr_test[i] = 0;
			arr[i] = i;
		}
		if (!testMCSP_1()) {
			printf("ERROR::MCSP_TEST_1\n");
			return 0;
		}
	}
    printf("SUCCESS::MCSP_TEST_1\n");

    for (int k = 0; k < CCnt1; ++k) {
        QueueMCSP<int> MCSPq;
        if (!testWriter_Readers<int>(&MCSPq)) {
            printf("ERROR::MCSP_TEST_Writer_Readers\n");
            return 0;
        }
    }
    printf("SUCCESS::MCSP_TEST_Writer_Readers\n");
    
    for (int k = 0; k < CCnt1; ++k) {
        QueueSCSP<int> SCSPq;
        if (!testWriter_Reader<int>(&SCSPq)) {
            printf("ERROR::SCSP_TEST_Writer_Reader\n");
            return 0;
        }
    }
    printf("SUCCESS::SCSP_TEST_Writer_Reader\n");
        
	//
	// TIME
	//
    int timeT = 0;
    QueueMCSP<int> MCSPq;
    QueueSCSP<int> SCSPq;

    timeT = 0;
    for (int k = 0; k < CCnt1; ++k)
        timeT += test_Time_Writer_Reader<int>(&MCSPq);
    printf("TIME::MCSP = %d\n", timeT/ CCnt1);
    
    timeT = 0;
    for (int k = 0; k < CCnt1; ++k)
        timeT += test_Time_Writer_Reader<int>(&SCSPq);
    printf("TIME::SCSP = %d\n", timeT/ CCnt1);
    
    return 0;
}

