#ifndef PERIODICA_H
#define	PERIODICA_H

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

//Estrutura que armazenara a informação da tarefa periódica

struct periodic_info {
    int sig;
    sigset_t alarm_sig;

};

//Configura sinal do timer
void configuraTimer();

//Configura período
int configuraPeriodica(int unsigned period, struct periodic_info *info);

//Função que aguarda pelo período
void aguardaSeg(int segundos, struct periodic_info *info);

//Obtem a hora do sistema
void obtemHora(char* buffer);

#endif	

