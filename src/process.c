// Grupo 013
// 60522 - João Rebelo
// 60807 - Juliana Duarte
// 59830 - Larissa Oliveira

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../include/main.h"
#include "../include/memory.h"
#include "../include/patient.h"
#include "../include/receptionist.h"
#include "../include/doctor.h"
#include "../include/process.h"
#include "../include/hospsignal.h"

/* Função que inicia um novo processo Paciente através da função fork do SO. O novo
* processo irá executar a função execute_patient respetiva, fazendo exit do retorno.
* O processo pai devolve o pid do processo criado.
*/
int launch_patient(int patient_id, struct data_container* data, struct communication* comm, struct semaphores* sems) {
    pid_t pid = fork();
    if(pid == -1){
        perror("fork-patient");
        exit(1);
    } else if(pid == 0){
        proper_process_termination();
        int adms = execute_patient(patient_id, data, comm,sems);
        exit(adms);
    } else {
        return pid;
    }
}

/* Função que inicia um novo processo Rececionista através da função fork do SO. O novo
* processo irá executar a função execute_receptionist, fazendo exit do retorno.
* O processo pai devolve o pid do processo criado.
*/
int launch_receptionist(int receptionist_id, struct data_container* data, struct communication* comm, struct semaphores* sems) {
    pid_t pid = fork();
    if(pid == -1){
        perror("fork-receptionist");
        exit(1);
    } else if(pid == 0){
        proper_process_termination();
        int adms = execute_receptionist(receptionist_id, data, comm, sems);
        exit(adms);
    } else {
        return pid;
    }
}

/* Função que inicia um novo processo Médico através da função fork do SO. O novo
* processo irá executar a função execute_doctor, fazendo exit do retorno.
* O processo pai devolve o pid do processo criado.
*/
int launch_doctor(int doctor_id, struct data_container* data, struct communication* comm, struct semaphores* sems) {
    pid_t pid = fork();
    if(pid == -1){
        perror("fork-doctor");
        exit(1);
    } else if(pid == 0){
        proper_process_termination();
        int adms = execute_doctor(doctor_id, data, comm, sems);
        exit(adms);
    } else {
        return pid;
    }
}

/* Função que espera que um processo termine através da função waitpid. 
* Devolve o retorno do processo, se este tiver terminado normalmente.
*/
int wait_process(int process_id) {
    int status;
    
    waitpid(process_id, &status, 0);

    if(WIFEXITED(status)) {
        return WEXITSTATUS(status);
    } else {
        perror("Processo não terminou normalmente");
        exit(1);   
    }     
}