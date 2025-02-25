// Grupo 013
// 60522 - João Rebelo
// 60807 - Juliana Duarte
// 59830 - Larissa Oliveira

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../include/memory.h"
#include "../include/hosptime.h"

#include <time.h>

/* Função que regista o tempo de criação de uma admissão
*/
void register_create_time(struct admission* ad) {
    if( clock_gettime(CLOCK_REALTIME, &(ad->create_time)) == -1 ) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }
}

/* Função que regista o tempo de receção de uma admissão por um paciente.
*/
void register_patient_time(struct admission* ad){
    if( clock_gettime(CLOCK_REALTIME, &(ad->patient_time)) == -1 ) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }
}

/* Função que regista o tempo de processamento de uma admissão por um rececionista
*/
void register_receptionist_time(struct admission* ad){
    if( clock_gettime(CLOCK_REALTIME, &(ad->receptionist_time)) == -1 ) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }
}

/* Função que regista o tempo de processamento de uma admissão por um médico.
*/
void register_doctor_time(struct admission* ad){
    if( clock_gettime(CLOCK_REALTIME, &(ad->doctor_time)) == -1 ) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }
}