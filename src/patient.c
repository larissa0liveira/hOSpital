// Grupo 013
// 60522 - João Rebelo
// 60807 - Juliana Duarte
// 59830 - Larissa Oliveira

#include <stdio.h>
#include "../include/main.h"
#include "../include/memory.h"
#include "../include/patient.h"
#include "../include/hosptime.h"

/* Função principal de um Paciente. Deve executar um ciclo infinito onde em
 * cada iteração lê uma admissão da main e se data->terminate ainda
 * for igual a 0, processa-a. Admissões com id igual a -1 são ignoradas
 * (ad inválida) e se data->terminate for igual a 1 é porque foi dada ordem
 * de terminação do programa, portanto deve-se fazer return do número de
 * admissões pedidas. Para efetuar estes passos, pode usar os outros
 * métodos auxiliares definidos em patient.h.
 */
int execute_patient(int patient_id, struct data_container *data, struct communication *comm, struct semaphores* sems)
{
    struct admission adm;
    struct admission* ad = &adm;

    semaphore_lock(sems->terminate_mutex);

    while (*(data->terminate) == 0) {

        semaphore_unlock(sems->terminate_mutex);
        patient_receive_admission(ad, patient_id, data, comm, sems);

        if (ad->id == -1) {
            semaphore_lock(sems->terminate_mutex);
            continue;
        }
        patient_process_admission(ad, patient_id, data, sems);
        patient_send_admission(ad, data, comm, sems);
        printf("[Patient %d] Recebi a admissão %d!\n", ad->receiving_patient, ad->id);

        semaphore_lock(sems->terminate_mutex);
    }

    semaphore_unlock(sems->terminate_mutex);

    return data->patient_stats[patient_id];

}


/* Função que lê uma admissão (do buffer de memória partilhada entre a main
 * e os pacientes) que seja direcionada a patient_id. Antes de tentar ler a admissão, deve
 * verificar se data->terminate tem valor 1. Em caso afirmativo, retorna imediatamente da função.
 */
void patient_receive_admission(struct admission *ad, int patient_id, struct data_container *data, struct communication *comm,struct semaphores* sems) 
{
    semaphore_lock(sems->terminate_mutex);
    if (*(data->terminate) == 1){
        semaphore_unlock(sems->terminate_mutex);
        return;
    }
    semaphore_unlock(sems->terminate_mutex);

    consume_begin(sems->main_patient);
    read_main_patient_buffer(comm->main_patient, patient_id, data->buffers_size, ad);
    consume_end(sems->main_patient);
    
    if(ad->id == -1){
        produce_begin(sems->main_patient);
        produce_end(sems->main_patient);
    }
}

/* Função que valida uma admissão, alterando o seu campo receiving_patient para o patient_id
 * passado como argumento, alterando o estado da mesma para 'P' (patient), e
 * incrementando o contador de admissões solicitadas por este paciente no data_container.
 * Atualiza também a admissão na estrutura data.
 */
void patient_process_admission(struct admission *ad, int patient_id, struct data_container *data,struct semaphores* sems)
{
    semaphore_lock(sems->terminate_mutex);
    if (*(data->terminate) == 1){
        semaphore_unlock(sems->terminate_mutex);
        return;
    }
    semaphore_unlock(sems->terminate_mutex);

    ad->receiving_patient = patient_id;
    ad->status = 'P';
    register_patient_time(ad);

    semaphore_lock(sems->patient_stats_mutex);
    data->patient_stats[patient_id]++;
    semaphore_unlock(sems->patient_stats_mutex);

    semaphore_lock(sems->results_mutex);
    data->results[ad->id] = *ad;
    semaphore_unlock(sems->results_mutex);
}

/* Função que escreve uma admissão no buffer de memória partilhada entre os
 * pacientes e os rececionistas.
 */
void patient_send_admission(struct admission *ad, struct data_container *data, struct communication *comm,struct semaphores* sems) 
{
    semaphore_lock(sems->terminate_mutex);
    if (*(data->terminate) == 1){
        semaphore_unlock(sems->terminate_mutex);
        return;
    }
    semaphore_unlock(sems->terminate_mutex);

    produce_begin(sems->patient_receptionist);
    write_patient_receptionist_buffer(comm->patient_receptionist, data->buffers_size, ad);
    produce_end(sems->patient_receptionist);
}
