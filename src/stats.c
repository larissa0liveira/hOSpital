// Grupo 013
// 60522 - João Rebelo
// 60807 - Juliana Duarte
// 59830 - Larissa Oliveira

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/main.h"

FILE *stats_file;

void patients_stats(struct data_container *data) {

    for (int i = 0; i < data->n_patients; i++){
        if(data->patient_stats[i] != 0) {
            fprintf(stats_file, "Patient %d requested %d admissions!\n", i, data->patient_stats[i]);
        }
    }
}

void receptionists_stats(struct data_container *data) {

    for (int i = 0; i < data->n_receptionists; i++){
        if(data->receptionist_stats[i] != 0)
            fprintf(stats_file, "Receptionist %d forwarded %d admissions!\n", i, data->receptionist_stats[i]);
    }
}

void doctors_stats(struct data_container *data) {

    for (int i = 0; i < data->n_doctors; i++){
        if(data->doctor_stats[i] != 0)
            fprintf(stats_file, "Doctor %d processed %d admissions!\n", i, data->doctor_stats[i]);
    }
}

void formatted_time(struct timespec *time_input) {
    struct tm *time_info;
    char formatted[25];
    long miliseconds;
    char str[50];

    time_info = localtime(&(time_input->tv_sec));
    strftime(formatted, 25, "%d/%m/%Y_%X", time_info);
    miliseconds = time_input->tv_nsec / 1000000;
    snprintf(str, 50, "%s.%03ld", formatted, miliseconds);
    fprintf(stats_file, "%s\n", str);
}

void total_adm_time(struct timespec *start, struct timespec *end) {
    struct timespec diff;

    if((end->tv_nsec - start->tv_nsec) > 0) {
        diff.tv_sec = end->tv_sec - start->tv_sec;
        diff.tv_nsec = end->tv_nsec - start->tv_nsec;
    } else {
        diff.tv_sec = end->tv_sec - start->tv_sec - 1;
        diff.tv_nsec = end->tv_nsec - start->tv_nsec + 1000000000;
    }

    long miliseconds = diff.tv_nsec / 1000000;

    char str[10];
    snprintf(str, 10, "%ld.%03ld", diff.tv_sec, miliseconds);
    fprintf(stats_file, "%s\n", str);
}

void admissions_stats(struct data_container *data) {

    struct admission ad;

    //int i = 0;
    // ad = data->results[i];

    for(int i = 0; i < data->max_ads; i++){
    ad = data->results[i];
    if(ad.status == 'M' || ad.status == 'P' || ad.status == 'R' || ad.status == 'A' || ad.status == 'N') {

        fprintf(stats_file, "Admission: %d\n", ad.id);
        fprintf(stats_file, "Status: %c\n", ad.status);

        if(ad.receiving_patient >= 0)
            fprintf(stats_file, "Patient id: %d\n", ad.receiving_patient);
        if(ad.receiving_receptionist >= 0)
            fprintf(stats_file, "Receptionist id: %d\n", ad.receiving_receptionist);
        if(ad.receiving_doctor >= 0)
            fprintf(stats_file, "Doctor id: %d\n", ad.receiving_doctor);

        fprintf(stats_file, "Start time: ");
        formatted_time(&ad.create_time);
        
        if(ad.patient_time.tv_sec > 0){
            fprintf(stats_file, "Patient time: ");
            formatted_time(&ad.patient_time);
        }
        if(ad.receptionist_time.tv_sec > 0){
            fprintf(stats_file, "Receptionist time: ");
            formatted_time(&ad.receptionist_time);
        }
        if(ad.doctor_time.tv_sec > 0){
            fprintf(stats_file, "Doctor time: "); 
            formatted_time(&ad.doctor_time);
            fprintf(stats_file, "Total Time: "); 
            total_adm_time(&ad.create_time, &ad.doctor_time); 
        }

        fprintf(stats_file, "\n");

        // i++;
        // ad = data->results[i];
    }
    }
}

/* Função que imprime as estatisticas finais do hOSpital,
* incluindo as estatísticas dos vários processos e das
* admissões.
 */
void write_stats_to_file(struct data_container *data) {

    stats_file = fopen(data->statistics_filename, "w");

    if (stats_file == NULL) {
        perror("fopen_write_stats");
        exit(1);
    }
    fprintf(stats_file, "Process statistics:\n");
    patients_stats(data);
    receptionists_stats(data);
    doctors_stats(data);
    fprintf(stats_file, "\n");

    fprintf(stats_file, "Admission statistics:\n");

    admissions_stats(data);

    fprintf(stats_file, "\n");

    fclose(stats_file);
}