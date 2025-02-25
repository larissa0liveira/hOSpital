// Grupo 013
// 60522 - João Rebelo
// 60807 - Juliana Duarte
// 59830 - Larissa Oliveira

#include <stdio.h>
#include "../include/memory.h"
#include "../include/main.h"
#include "../include/doctor.h"
#include "../include/hosptime.h"

/* Função principal de um Médico. Deve executar um ciclo infinito onde em
 * cada iteração lê uma admissão e se data->terminate ainda for igual a 0, processa-a e
 * escreve-a para os resultados. Admissões com id igual a -1 são ignoradas
 * (ad inválida) e se data->terminate for igual a 1 é porque foi dada ordem
 * de terminação do programa, portanto deve-se fazer return do número de
 * consultas realizadas. Para efetuar estes passos, pode usar os outros
 * métodos auxiliares definidos em doctor.h.
 */
int execute_doctor(int doctor_id, struct data_container *data, struct communication *comm, struct semaphores* sems) {

    struct admission adm;
    struct admission* ad = &adm;

    semaphore_lock(sems->terminate_mutex);
    while (*(data->terminate) == 0) {

        semaphore_unlock(sems->terminate_mutex);
        doctor_receive_admission(ad, doctor_id, data, comm, sems);

        if (ad->id == -1){
            semaphore_lock(sems->terminate_mutex);
            continue;
        }
        doctor_process_admission(ad, doctor_id, data, sems);
        printf("[Doctor %d] Recebi a admissão %d!\n", ad->receiving_doctor, ad->id);

        semaphore_lock(sems->terminate_mutex);
    }
    semaphore_unlock(sems->terminate_mutex);
    return data->doctor_stats[doctor_id];
}

/* Função que lê uma admissão (do buffer de memória partilhada entre
 * os rececionistas e os médicos) que seja direcionada ao médico doctor_id.
 * Antes de tentar ler a admissão, o processo deve
 * verificar se data->terminate tem valor 1.
 * Em caso afirmativo, retorna imediatamente da função.
 */
void doctor_receive_admission(struct admission *ad, int doctor_id, struct data_container *data, struct communication *comm, struct semaphores* sems) 
{

    semaphore_lock(sems->terminate_mutex);
    if (*(data->terminate) == 1) {
        semaphore_unlock(sems->terminate_mutex);
        return;
    }
    semaphore_unlock(sems->terminate_mutex);

    consume_begin(sems->receptionist_doctor);
    read_receptionist_doctor_buffer(comm->receptionist_doctor, doctor_id, data->buffers_size, ad);
    consume_end(sems->receptionist_doctor);

    if(ad->id == -1){
        produce_begin(sems->receptionist_doctor);
        produce_end(sems->receptionist_doctor);
    }
}

/* Função que processa uma admissão, alterando o seu campo receiving_doctor para o id
 * passado como argumento, alterando o estado da mesma para 'A' e
 * incrementando o contador de consultas no data_container ou para 'N'.
 * Atualiza também a admissão na estrutura data.
 */
void doctor_process_admission(struct admission *ad, int doctor_id, struct data_container *data, struct semaphores* sems)  
{

    semaphore_lock(sems->terminate_mutex);
    if (*(data->terminate) == 1){
        semaphore_unlock(sems->terminate_mutex);
        return;
    }
    semaphore_unlock(sems->terminate_mutex);

    ad->receiving_doctor = doctor_id;
    register_doctor_time(ad);

    if (ad->id < data->max_ads){
        ad->status = 'A';

        semaphore_lock(sems->doctor_stats_mutex);
        data->doctor_stats[doctor_id]++;
        semaphore_unlock(sems->doctor_stats_mutex);
    } 
    else
        ad->status = 'N';

    semaphore_lock(sems->results_mutex);
    data->results[ad->id] = *ad;
    semaphore_unlock(sems->results_mutex);
}
