// Grupo 013
// 60522 - João Rebelo
// 60807 - Juliana Duarte
// 59830 - Larissa Oliveira

#include <stdio.h>
#include <unistd.h> 
#include "../include/memory.h"
#include "../include/main.h"
#include "../include/receptionist.h"
#include "../include/hosptime.h"

/* Função principal de um Rececionista. Deve executar um ciclo infinito onde em
 * cada iteração lê uma admissão dos pacientes e se a mesma tiver id
 * diferente de -1 e se data->terminate ainda for igual a 0, processa-a e
 * a encaminha para os médicos. Admissões com id igual a -1 são
 * ignoradas (ad inválida) e se data->terminate for igual a 1 é porque foi
 * dada ordem de terminação do programa, portanto deve-se fazer return do
 * número de admissões realizadas. Para efetuar estes passos, pode usar os
 * outros métodos auxiliares definidos em receptionist.h.
 */
int execute_receptionist(int receptionist_id, struct data_container *data, struct communication *comm, struct semaphores* sems) 
{
    struct admission adm;
    struct admission* ad = &adm;

    semaphore_lock(sems->terminate_mutex);
    
    while (*(data->terminate) == 0){

        semaphore_unlock(sems->terminate_mutex);
        receptionist_receive_admission(ad, data, comm, sems);

        if (ad->id == -1) {
            semaphore_lock(sems->terminate_mutex);
            continue;
        }
        receptionist_process_admission(ad, receptionist_id, data,sems);
        receptionist_send_admission(ad, data, comm,sems);

        printf("[Receptionist %d] Recebi a admissão %d!\n", ad->receiving_receptionist, ad->id);

        semaphore_lock(sems->terminate_mutex);
    }

    semaphore_unlock(sems->terminate_mutex);

    return data->receptionist_stats[receptionist_id];
}


/* Função que lê uma admissão do buffer de memória partilhada entre os pacientes e os rececionistas.
 * Antes de tentar ler a admissão, deve verificar se data->terminate tem valor 1.
 * Em caso afirmativo, retorna imediatamente da função.
 */
void receptionist_receive_admission(struct admission *ad, struct data_container *data, struct communication *comm, struct semaphores* sems)
{
    semaphore_lock(sems->terminate_mutex);
    if (*(data->terminate) == 1){
        semaphore_unlock(sems->terminate_mutex);
        return;
    }
    semaphore_unlock(sems->terminate_mutex);

    consume_begin(sems->patient_receptionist);
    read_patient_receptionist_buffer(comm->patient_receptionist, data->buffers_size, ad);
    consume_end(sems->patient_receptionist);
}

/* Função que realiza uma admissão, alterando o seu campo receiving_receptionist para o id
 * passado como argumento, alterando o estado da mesma para 'R' (receptionist), e
 * incrementando o contador de admissões realizadas por este rececionista no data_container.
 * Atualiza também a admissão na estrutura data.
 */
void receptionist_process_admission(struct admission *ad, int receptionist_id, struct data_container *data, struct semaphores* sems)
{

    semaphore_lock(sems->terminate_mutex);
    if (*(data->terminate) == 1){
        semaphore_unlock(sems->terminate_mutex);
        return;
    }
    semaphore_unlock(sems->terminate_mutex);
    
    ad->receiving_receptionist = receptionist_id;
    ad->status = 'R';
    register_receptionist_time(ad);

    semaphore_lock(sems->receptionist_stats_mutex);
    data->receptionist_stats[receptionist_id]++;
    semaphore_unlock(sems->receptionist_stats_mutex);

    semaphore_lock(sems->results_mutex);
    data->results[ad->id] = *ad;
    semaphore_unlock(sems->results_mutex);

}

/* Função que escreve uma admissão no buffer de memória partilhada entre
 * os rececionistas e os médicos.
 */
void receptionist_send_admission(struct admission *ad, struct data_container *data, struct communication *comm, struct semaphores* sems) 
{
    semaphore_lock(sems->terminate_mutex);
    if (*(data->terminate) == 1) {
        semaphore_unlock(sems->terminate_mutex);
        return;
    }
    semaphore_unlock(sems->terminate_mutex);

    if(ad->requested_doctor < data->n_doctors) {
        produce_begin(sems->receptionist_doctor);
        write_receptionist_doctor_buffer(comm->receptionist_doctor, data->buffers_size, ad);
        produce_end(sems->receptionist_doctor);
    }
}
