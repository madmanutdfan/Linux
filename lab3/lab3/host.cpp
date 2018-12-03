#include <cstdlib>
#include <cstdio>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <ctime>
#include <sys/stat.h>
#include <unistd.h>
#include <mqueue.h>

#include"semaphore.h"
#include"goatlings.h"
#include"conn.h"

using namespace std;

void  host    ();
void  client  ();

int   pid      = 0;
int   gameover = 0;

int main(int argc, char *argv[])
{
	openlog("Goatlings", LOG_PID | LOG_CONS, LOG_USER);

	sem_host     = sem_open(SemaphoreHost, O_CREAT, 0777, 1);
	sem_rw       = sem_open(SemaphoreRW, O_CREAT, 0777, 0);
	sem_gameover = sem_open(SemaphoreGO, O_CREAT, 0777, 0);

	for (int i = 0; i < nGoatlings; i++) {
		from_host[i] = new Conn(i, true);
		syslog(LOG_INFO, "HOST::connection\n")
	}

	sem_wait(sem_host);

	for (int i = 0; i < nGoatlings; i++) {
		int pid_fork = fork();

		if (pid_fork != 0) {
			pid = i + 1;
			Conn::Create(i);
			break;
		}
	}

	while (gameover < 2) {
		if (pid > 0)
			client();

		sem_wait(sem_host);
		sem_post(sem_host);

		if (pid == 0)
			host();
	}
	syslog(LOG_INFO, "END::ALL GOATLINGS ARE DEAD\n");

	if (pid == 0) {
		sem_close(sem_host);
		sem_unlink(SemaphoreHost);
		sem_close(sem_rw);
		sem_unlink(SemaphoreRW);
		sem_close(sem_gameover);
		sem_unlink(SemaphoreGO);
		for (int i = 0; i < nGoatlings; i++) {
			delete(from_host[i]);
			delete(from_client[i]);
		}
		closelog();
	}
    return 0;
}

void host() {
	int numberWolf = rand() % 100 + 1;
	int numberGoatling;
	int N = 70 / nGoatlings;
	int n = 20 / nGoatlings;

	from_client[pid - 1]->Read((int*)numberGoatling, 1); // считываем число козленка из fd[0][0]

	if (goatlings[pid - 1]->getState == ALIVE) {// записываем число в fd[1][1]
		if (numberGoatling - N > numberWolf || numberGoatling + N < numberWolf)
			from_host[pid - 1]->Write((int*)DEAD, 1); 
		else 
			from_host[pid - 1]->Write((int*)ALIVE, 1);
	}
	else {
		if (numberGoatling - n < numberWolf && numberGoatling + n > numberWolf)
			from_host[pid - 1]->Write((int*)ALIVE, 1);
		else 
			from_host[pid - 1]->Write((int*)DEAD, 1);
	}

	sem_post(sem_rw);

	for (int i = 0; i < nGoatlings; i++)
		sem_wait(sem_gameover);
	if (nDead == nGoatlings)
		++gameover;
}

void client() {
	status state;
	from_client[pid - 1]->Write((int*)goatlings[pid - 1]->getNumber, 1);// записываем число козлёнка в fd[0][1]

	sem_post(sem_host);

	sem_wait(sem_rw);

	from_host[pid -1]->Read((int*)state, 1);
	goatlings[pid - 1]->setState(state); // + еще там считаем число мертвых

	sem_post(sem_gameover);
}