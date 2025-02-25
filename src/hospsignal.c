// Grupo 013
// 60522 - João Rebelo
// 60807 - Juliana Duarte
// 59830 - Larissa Oliveira

#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include "../include/hospsignal.h"

struct data_container* dados;
struct communication* comunicacoes;
struct semaphores* semaforos;
struct itimerval val;

/* Função que inicia o tratamento de sinais e o temporizador de alarmes
*/
void start_alerts(struct data_container* data, struct communication* comm, struct semaphores* sems){

    signal(SIGINT, end_signal);
	signal(SIGALRM, print_current_status);

    dados = data;
    comunicacoes = comm;
    semaforos = sems;

	val.it_interval.tv_sec = dados->alarm_time; // tempo ate primeira expiracao em s
	val.it_interval.tv_usec = 0; // tempo ate primeira expiracao em ms
	val.it_value.tv_sec = dados->alarm_time; // tempo entre expiracoes em s
	val.it_value.tv_usec = 0; // tempo entre expiracoes em ms
	setitimer(ITIMER_REAL, &val, 0);
}

void end_signal(){
    end_execution(dados, comunicacoes, semaforos);
    exit(0);
}

void proper_process_termination(){
    signal(SIGINT, SIG_IGN);
}

void print_current_status()
{   
    char mainPrintBuffer[256];
    char printBuffer[100];
    struct admission current;

    semaphore_lock(semaforos->results_mutex);
    for(int i = 0; i < dados->max_ads; i++){

        current = dados->results[i];
        sprintf(mainPrintBuffer, "ad:%d status: %c", i, current.status);

        if(current.create_time.tv_sec > 0){
            sprintf(printBuffer,  " start_time: %ld", current.create_time.tv_sec);
            strncat(mainPrintBuffer, printBuffer, 100);
        } else {
            strcat(mainPrintBuffer, "{-disponível-}");
        }
        if(current.patient_time.tv_sec > 0){
            sprintf(printBuffer, " patient:%d patient_time: %ld", current.receiving_patient, current.patient_time.tv_sec);
            strncat(mainPrintBuffer, printBuffer, 100);
        }
        if(current.receptionist_time.tv_sec > 0){
            sprintf(printBuffer, " receptionist:%d receptionist_time: %ld", current.receiving_receptionist, current.receptionist_time.tv_sec);
            strncat(mainPrintBuffer, printBuffer, 100);
        }
        if(current.doctor_time.tv_sec > 0){
            sprintf(printBuffer, " doctor:%d doctor_time: %ld", current.receiving_doctor, current.doctor_time.tv_sec);
            strncat(mainPrintBuffer, printBuffer, 100);
        }
        printf("%s\n",mainPrintBuffer);
    }
    semaphore_unlock(semaforos->results_mutex);

	signal(SIGALRM, print_current_status);
}