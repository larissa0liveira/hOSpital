// Grupo 013
// 60522 - João Rebelo
// 60807 - Juliana Duarte
// 59830 - Larissa Oliveira

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/main.h"
#include "../include/memory.h"
#include "../include/process.h"
#include "../include/hosptime.h"
#include "../include/synchronization.h"
#include "../include/log.h"
#include "../include/hospsignal.h"
#include "../include/configuration.h"
#include "../include/stats.h"

int ad_counter = 0;


int main(int argc, char *argv[]) {

//init data structures
struct data_container* data = allocate_dynamic_memory(sizeof(struct data_container));
struct communication* comm = allocate_dynamic_memory(sizeof(struct communication));

comm->main_patient = allocate_dynamic_memory(sizeof(struct circular_buffer));
comm->patient_receptionist = allocate_dynamic_memory(sizeof(struct rnd_access_buffer));
comm->receptionist_doctor = allocate_dynamic_memory(sizeof(struct circular_buffer));

// init semaphore data structure
struct semaphores* sems = allocate_dynamic_memory(sizeof(struct semaphores));
sems->main_patient = allocate_dynamic_memory(sizeof(struct prodcons));
sems->patient_receptionist = allocate_dynamic_memory(sizeof(struct prodcons));
sems->receptionist_doctor = allocate_dynamic_memory(sizeof(struct prodcons));

//execute main code
main_args(argc, argv, data);
allocate_dynamic_memory_buffers(data);
create_shared_memory_buffers(data, comm);
create_semaphores(data, sems);
launch_processes(data, comm, sems);
user_interaction(data, comm, sems);

}

/* Função que lê os argumentos da aplicação, nomeadamente o número
 * máximo de admissões, o tamanho dos buffers de memória partilhada
 * usados para comunicação, e o número de pacientes, de rececionistas e de
 * médicos. Guarda esta informação nos campos apropriados da
 * estrutura data_container.
 */
void main_args(int argc, char *argv[], struct data_container *data) {

    if (argc != 2) {
        fprintf(stderr, "Utilizacao:\n%s <nome ficheiro>\n", argv[0]);
        exit(1);
    }
    read_configuration(data, argv[1]);
}

/* Função que reserva a memória dinâmica necessária para a execução
 * do hOSpital, nomeadamente para os arrays *_pids da estrutura
 * data_container. Para tal, pode ser usada a função allocate_dynamic_memory.
 */
void allocate_dynamic_memory_buffers(struct data_container *data) {

    data->patient_pids = allocate_dynamic_memory(data->n_patients * sizeof(int));
    data->receptionist_pids = allocate_dynamic_memory(data->n_receptionists * sizeof(int));
    data->doctor_pids = allocate_dynamic_memory(data->n_doctors * sizeof(int));

}

/* Função que reserva a memória partilhada necessária para a execução do
 * hOSpital. É necessário reservar memória partilhada para todos os buffers da
 * estrutura communication, incluindo os buffers em si e respetivos
 * pointers, assim como para o array data->results e variável data->terminate.
 * Para tal, pode ser usada a função create_shared_memory.
 */
void create_shared_memory_buffers(struct data_container *data, struct communication *comm) {

    // Reserva memória partilhada entre a main e patient
    comm->main_patient->ptrs = create_shared_memory(STR_SHM_MAIN_PATIENT_PTR, sizeof(struct pointers));
    comm->main_patient->buffer = create_shared_memory(STR_SHM_MAIN_PATIENT_BUFFER, data->buffers_size * sizeof(struct admission));

    // Reserva memória partilhada entre a patient e receptionist
    comm->patient_receptionist->ptrs = create_shared_memory(STR_SHM_PATIENT_RECEPT_PTR, data->buffers_size * sizeof(int));
    comm->patient_receptionist->buffer = create_shared_memory(STR_SHM_PATIENT_RECEPT_BUFFER, data->buffers_size * sizeof(struct admission));

    // Reserva memória partilhada entre a receptionist e doctor
    comm->receptionist_doctor->ptrs = create_shared_memory(STR_SHM_RECEPT_DOCTOR_PTR, sizeof(struct pointers));
    comm->receptionist_doctor->buffer = create_shared_memory(STR_SHM_RECEPT_DOCTOR_BUFFER, data->buffers_size * sizeof(struct admission));

    data->patient_stats = create_shared_memory(STR_SHM_PATIENT_STATS, data->n_patients * sizeof(int));
    data->receptionist_stats = create_shared_memory(STR_SHM_RECEPT_STATS, data->n_receptionists * sizeof(int));
    data->doctor_stats = create_shared_memory(STR_SHM_DOCTOR_STATS, data->n_doctors * sizeof(int));
    data->results = create_shared_memory(STR_SHM_RESULTS, MAX_RESULTS * sizeof(struct admission));
    data->terminate = create_shared_memory(STR_SHM_TERMINATE, sizeof(int));
}

/* Função que inicia os processos dos pacientes, rececionistas e
 * médicos. Para tal, pode usar as funções launch_*,
 * guardando os pids resultantes nos arrays respetivos
 * da estrutura data.
 */
void launch_processes(struct data_container *data, struct communication *comm, struct semaphores* sems) {

    for (int i = 0; i < data->n_patients; i++){
        data->patient_pids[i] = launch_patient(i, data, comm, sems);
    }
    for (int i = 0; i < data->n_receptionists; i++){
        data->receptionist_pids[i] = launch_receptionist(i, data, comm, sems);
    }
    for (int i = 0; i < data->n_doctors; i++){
        data->doctor_pids[i] = launch_doctor(i, data, comm, sems);
    }
}

void print_help() {
    printf("Ações disponíveis:\n");
    printf("ad paciente médico - criar uma nova admissão\n");
    printf("info id - consultar o estado de uma admissão\n");
    printf("status - apresentar o estado atual do hospital\n");
    printf("help - imprime informação sobre as ações disponíveis\n");
    printf("end - termina a execução do hOSpital.\n");
}

/* Função que faz a interação do utilizador, podendo receber 4 comandos:
 * ad paciente médico - cria uma nova admissão, através da função create_request
 * info - verifica o estado de uma admissão através da função read_info
 * help - imprime informação sobre os comandos disponiveis
 * end - termina o execução do hOSpital através da função stop_execution
 */
void user_interaction(struct data_container *data, struct communication *comm, struct semaphores* sems) {

    print_help();
    start_alerts(data, comm, sems);
    setup_log(data->log_filename);

    char input[MAX_RESULTS];

    //não precisa de semáforo, porque só existe um main e nenhum dos filhos escreve no terminate
    while(*(data->terminate) == 0){
        printf("\nInsira a sua opção: ");
        scanf("%s", input);

        if(strcmp(input, "help")==0){
            print_help();   
                               
        } else if (strcmp(input, "status")==0) {
            printf("\nO estado atual do hOSpital:\n");
            print_status(data, sems);

        } else if (strcmp(input, "end")==0) {
            printf("Terminando a execucao...\n");
            end_execution(data, comm, sems);
            break;

        } else if (strcmp(input, "info")==0) {
            read_info(data, sems);

        } else if (strcmp(input, "ad")==0) {
            create_request(&ad_counter, data, comm, sems);
            // sleep(1);
        } else {
            printf("Comando inválido, insira 'help' para ajuda");
        }
    }
}

/* Cria uma nova admissão identificada pelo valor atual de ad_counter e com os
 * dados introduzidos pelo utilizador na linha de comandos, escrevendo a mesma
 * no buffer de memória partilhada entre a main e os pacientes. Imprime o id da
 * admissão e incrementa o contador de admissões ad_counter.
 */
void create_request(int *ad_counter, struct data_container *data, struct communication *comm, struct semaphores* sems) {
    int patient_id;
    int doctor_id;
    char operation[20];
    scanf("%d %d", &patient_id, &doctor_id);

    snprintf(operation, sizeof(operation), "ad %d %d", patient_id, doctor_id);
    write_to_log(operation);

    if (*(ad_counter) >= data->max_ads){
        printf("O número máximo de admissões foi alcançado!\n");
    }
    struct admission ad;
    ad.id = *ad_counter;
    ad.requesting_patient = patient_id;
    ad.requested_doctor = doctor_id;
    ad.status = 'M';

    ad.receiving_patient = -1;		//id do paciente que recebeu a admissão
	ad.receiving_receptionist = -1;	//id do rececionista que realizou a admissão
	ad.receiving_doctor = -1;	
    
    ad.patient_time.tv_sec = 0;
    ad.patient_time.tv_nsec = 0;
    ad.receptionist_time.tv_sec = 0;
    ad.receptionist_time.tv_nsec = 0;
    ad.doctor_time.tv_sec = 0;
    ad.doctor_time.tv_nsec = 0;
    register_create_time(&ad);

    semaphore_lock(sems->results_mutex);
    data->results[*ad_counter] = ad;
    semaphore_unlock(sems->results_mutex);

    printf("Admissão criada! ID da admissão: %d\n", *ad_counter);

    if(patient_id < data->n_patients){
        produce_begin(sems->main_patient);
        write_main_patient_buffer(comm->main_patient, data->buffers_size, &ad);
        produce_end(sems->main_patient);
    }

    (*ad_counter)++;
}

/* Função que lê um id de admissão do utilizador e verifica se a mesma é valida.
 * Em caso afirmativo imprime a informação da mesma, nomeadamente o seu estado, o
 * id do paciente que fez o pedido, o id do médico requisitado, e os ids do paciente,
 * rececionista, e médico que a receberam e processaram.
 */
void read_info(struct data_container *data, struct semaphores* sems) {
    
    int admission_id;
    scanf("%d", &admission_id);

    char operation[20];
    snprintf(operation, sizeof(operation), "info %d", admission_id);
    write_to_log(operation);

    if (admission_id < 0) {
        printf("ID de admissão inválido.\n");
        return;
    }

    semaphore_lock(sems->results_mutex); 
    struct admission ad = data->results[admission_id];

    char prefab[MAX_RESULTS]; 
    sprintf(prefab, "A admissão %d com estado %c requisitada pelo paciente %d ao médico %d, ", ad.id, ad.status, ad.requesting_patient, ad.requested_doctor);

    switch (ad.status) {
        case 'M':
        printf("%sfoi enviada ao paciente!\n", prefab);
        break;
        case 'P':
        printf("%sfoi recebida pelo paciente %d e aguarda rececionista!\n", prefab, ad.receiving_patient);
        break;
        case 'R':
        printf("%sfoi recebida pelo paciente %d, e admitida pelo rececionista %d!\n", prefab, ad.receiving_patient, ad.receiving_receptionist);
        break;
        case 'A':
        printf("%sfoi recebida pelo paciente %d, admitida pelo rececionista %d e concluída pelo médico %d!\n", prefab, ad.receiving_patient, ad.receiving_receptionist, ad.receiving_doctor);
        break;
        case 'N':
        printf("%sfoi recebida pelo paciente %d, admitida pelo rececionista %d e remarcada pelo médico %d!\n", prefab, ad.receiving_patient, ad.receiving_receptionist, ad.receiving_doctor);
        break;
        default:
        printf("A admissão %d ainda não existe!\n", admission_id);
    }
    semaphore_unlock(sems->results_mutex);

}

// função auxiliar
void print_array(int *array, int size) {

    printf("[%d", array[0]);
    for (int i = 1; i < size; i++)
    {
        printf(", %d", array[i]);
    }
    printf("] \n");
}

/* Função que imprime o estado do data_container, nomeadamente todos os seus campos.
 * No caso dos arrays, deve-se imprimir no formato [0, 1, 2, ..., N], onde N é o último elemento do array.
 */
void print_status(struct data_container *data, struct semaphores* sems) {

    write_to_log("status");

    printf("Número máximo de admissões: %d\n", data->max_ads);
    printf("Tamanho máximo dos buffers: %d\n", data->buffers_size);

    printf("Número de pacientes: %d\n", data->n_patients);
    printf("Número de rececionistas: %d\n", data->n_receptionists);
    printf("Número de médicos: %d\n", data->n_doctors);

    printf("PIDs dos pacientes: ");
    print_array(data->patient_pids, data->n_patients);
    printf("PIDs dos rececionistas: ");
    print_array(data->receptionist_pids, data->n_receptionists);
    printf("PIDs dos médicos: ");
    print_array(data->doctor_pids, data->n_doctors);
    
    semaphore_lock(sems->patient_stats_mutex);
    printf("Stats dos pacientes: ");
    print_array(data->patient_stats, data->n_patients);
    semaphore_unlock(sems->patient_stats_mutex);

    semaphore_lock(sems->receptionist_stats_mutex);
    printf("Stats dos rececionistas: ");
    print_array(data->receptionist_stats, data->n_receptionists);
    semaphore_unlock(sems->receptionist_stats_mutex);

    semaphore_lock(sems->doctor_stats_mutex);
    printf("Stats dos médicos: ");
    print_array(data->doctor_stats, data->n_doctors);
    semaphore_unlock(sems->doctor_stats_mutex);

    semaphore_lock(sems->results_mutex);
    printf("Histórico de operações:\n");
    for (int i = 0; i < ad_counter; i++) {
        
        struct admission *ad = &(data->results[i]);
        printf("Admissão %d: [Requisitante: %d, Médico: %d, Status: %c, Paciente: %d, Recepcionista: %d, Médico: %d ]\n",
               i, ad->requesting_patient, ad->requested_doctor, ad->status, ad->receiving_patient, ad->receiving_receptionist, ad->receiving_doctor);
    }
    semaphore_unlock(sems->results_mutex);

    semaphore_lock(sems->terminate_mutex);
    printf("Terminar execução? %s\n", *(data->terminate) == 1 ? "Sim" : "Não");
    semaphore_unlock(sems->terminate_mutex);

}


/* Função que termina a execução do programa hOSpital. Deve começar por
 * afetar a flag data->terminate com o valor 1. De seguida, e por esta
 * ordem, deve esperar que os processos filho terminem, deve escrever as
 * estatisticas finais do programa, e por fim libertar
 * as zonas de memória partilhada e dinâmica previamente
 * reservadas. Para tal, pode usar as outras funções auxiliares do main.h.
 */
void end_execution(struct data_container *data, struct communication *comm, struct semaphores* sems) {

    close_log();
    
    semaphore_lock(sems->terminate_mutex);
    *(data->terminate) = 1;
    semaphore_unlock(sems->terminate_mutex);
    
    wakeup_processes(data, sems);
    wait_processes(data);

    //Já não há processos filhos, semáforos não são mais necessários
    write_statistics(data);
    write_stats_to_file(data);

    destroy_memory_buffers(data, comm);
    destroy_semaphores(sems);
    deallocate_dynamic_memory(data);
    deallocate_dynamic_memory(comm->main_patient);
    deallocate_dynamic_memory(comm->patient_receptionist);
    deallocate_dynamic_memory(comm->receptionist_doctor);
    deallocate_dynamic_memory(comm);
    deallocate_dynamic_memory(sems->main_patient);
    deallocate_dynamic_memory(sems->patient_receptionist);
    deallocate_dynamic_memory(sems->receptionist_doctor);
    deallocate_dynamic_memory(sems);
}

/* Função que espera que todos os processos previamente iniciados terminem,
 * incluindo pacientes, rececionistas e médicos. Para tal, pode usar a função
 * wait_process do process.h.
 */
void wait_processes(struct data_container *data) {

    for (int i = 0; i < data->n_patients; i++) {
        wait_process(data->patient_pids[i]);
    }
    for (int i = 0; i < data->n_receptionists; i++) {
        wait_process(data->receptionist_pids[i]);
    }
    for (int i = 0; i < data->n_doctors; i++) {
        wait_process(data->doctor_pids[i]);
    }
}

/* Função que imprime as estatisticas finais do hOSpital, nomeadamente quantas
 * admissões foram solicitadas por cada paciente, realizadas por cada rececionista
 * e atendidas por cada médico.
 */
void write_statistics(struct data_container *data) {

    for (int i = 0; i < data->n_patients; i++){
        printf("O paciente %d recebeu %d senhas!\n", i, data->patient_stats[i]);
    }

    for (int i = 0; i < data->n_receptionists; i++){
        printf("O rececionista %d realizou %d admissões!\n", i, data->receptionist_stats[i]);
    }

    for (int i = 0; i < data->n_doctors; i++) {
        printf("O médico %d atendeu a %d senhas!\n", i, data->doctor_stats[i]);
    }
    printf("\n");
}

/* Função que liberta todos os buffers de memória dinâmica e partilhada previamente
 * reservados na estrutura data.
 */
void destroy_memory_buffers(struct data_container *data, struct communication *comm) {

    deallocate_dynamic_memory(data->patient_pids);
    deallocate_dynamic_memory(data->receptionist_pids);
    deallocate_dynamic_memory(data->doctor_pids);

    destroy_shared_memory(STR_SHM_PATIENT_STATS, data->patient_stats, data->n_patients * sizeof(int));
    destroy_shared_memory(STR_SHM_RECEPT_STATS, data->receptionist_stats, data->n_receptionists * sizeof(int));
    destroy_shared_memory(STR_SHM_DOCTOR_STATS, data->doctor_stats, data->n_doctors * sizeof(int));

    destroy_shared_memory(STR_SHM_MAIN_PATIENT_PTR, comm->main_patient->ptrs, sizeof(struct pointers));
    destroy_shared_memory(STR_SHM_MAIN_PATIENT_BUFFER, comm->main_patient->buffer, data->buffers_size * sizeof(struct admission));

    destroy_shared_memory(STR_SHM_PATIENT_RECEPT_PTR, comm->patient_receptionist->ptrs, data->buffers_size * sizeof(int));
    destroy_shared_memory(STR_SHM_PATIENT_RECEPT_BUFFER, comm->patient_receptionist->buffer, data->buffers_size * sizeof(struct admission));

    destroy_shared_memory(STR_SHM_RECEPT_DOCTOR_PTR, comm->receptionist_doctor->ptrs, sizeof(struct pointers));
    destroy_shared_memory(STR_SHM_RECEPT_DOCTOR_BUFFER, comm->receptionist_doctor->buffer, data->buffers_size * sizeof(struct admission));

    destroy_shared_memory(STR_SHM_RESULTS, data->results, MAX_RESULTS * sizeof(struct admission));
    destroy_shared_memory(STR_SHM_TERMINATE, data->terminate, sizeof(int));
}

/* Função que inicializa os semáforos da estrutura semaphores. Semáforos
* *_full devem ser inicializados com valor 0, semáforos *_empty com valor
* igual ao tamanho dos buffers de memória partilhada, e os *_mutex com
* valor igual a 1. Para tal pode ser usada a função semaphore_create.*/
void create_semaphores(struct data_container* data, struct semaphores* sems){
    sems->main_patient->full = semaphore_create(STR_SEM_MAIN_PATIENT_FULL, 0);
    sems->main_patient->empty = semaphore_create(STR_SEM_MAIN_PATIENT_EMPTY, data->buffers_size);
    sems->main_patient->mutex = semaphore_create(STR_SEM_MAIN_PATIENT_MUTEX, 1);

    sems->patient_receptionist->full = semaphore_create(STR_SEM_PATIENT_RECEPT_FULL, 0);
    sems->patient_receptionist->empty = semaphore_create(STR_SEM_PATIENT_RECEPT_EMPTY, data->buffers_size);
    sems->patient_receptionist->mutex = semaphore_create(STR_SEM_PATIENT_RECEPT_MUTEX, 1);

    sems->receptionist_doctor->full = semaphore_create(STR_SEM_RECEPT_DOCTOR_FULL, 0);
    sems->receptionist_doctor->empty = semaphore_create(STR_SEM_RECEPT_DOCTOR_EMPTY, data->buffers_size);
    sems->receptionist_doctor->mutex = semaphore_create(STR_SEM_RECEPT_DOCTOR_MUTEX, 1);

    sems->patient_stats_mutex = semaphore_create(STR_SEM_PATIENT_STATS_MUTEX, 1);
    sems->receptionist_stats_mutex = semaphore_create(STR_SEM_RECEPT_STATS_MUTEX, 1);
    sems->doctor_stats_mutex = semaphore_create(STR_SEM_DOCTOR_STATS_MUTEX, 1);
    sems->results_mutex = semaphore_create(STR_SEM_RESULTS_MUTEX, 1);
    sems->terminate_mutex = semaphore_create(STR_SEM_TERMINATE_MUTEX, 1);
}


/* Função que acorda todos os processos adormecidos em semáforos, para que
* estes percebam que foi dada ordem de terminação do programa. Para tal,
* pode ser usada a função produce_end sobre todos os conjuntos de semáforos
* onde possam estar processos adormecidos e um número de vezes igual ao
* máximo de processos que possam lá estar.*/
void wakeup_processes(struct data_container* data, struct semaphores* sems){
    
    for (int i = 0; i < data->n_patients; i++) {
        produce_end(sems->main_patient);
    }
    for (int i = 0; i < data->n_receptionists; i++) { 
        produce_end(sems->patient_receptionist);
    }
    for (int i = 0; i < data->n_doctors; i++) { 
        produce_end(sems->receptionist_doctor);
    }
} 

/* Função que liberta todos os semáforos da estrutura semaphores. */
void destroy_semaphores(struct semaphores* sems){
    
    semaphore_destroy(STR_SEM_MAIN_PATIENT_FULL, sems->main_patient->full);
    semaphore_destroy(STR_SEM_MAIN_PATIENT_EMPTY, sems->main_patient->empty);
    semaphore_destroy(STR_SEM_MAIN_PATIENT_MUTEX, sems->main_patient->mutex);

    semaphore_destroy(STR_SEM_PATIENT_RECEPT_FULL, sems->patient_receptionist->full);
    semaphore_destroy(STR_SEM_PATIENT_RECEPT_EMPTY, sems->patient_receptionist->empty);
    semaphore_destroy(STR_SEM_PATIENT_RECEPT_MUTEX, sems->patient_receptionist->mutex);

    semaphore_destroy(STR_SEM_RECEPT_DOCTOR_FULL, sems->receptionist_doctor->full);
    semaphore_destroy(STR_SEM_RECEPT_DOCTOR_EMPTY, sems->receptionist_doctor->empty);
    semaphore_destroy(STR_SEM_RECEPT_DOCTOR_MUTEX, sems->receptionist_doctor->mutex);

     
    semaphore_destroy(STR_SEM_PATIENT_STATS_MUTEX, sems->patient_stats_mutex);
    semaphore_destroy(STR_SEM_RECEPT_STATS_MUTEX, sems->receptionist_stats_mutex);
    semaphore_destroy(STR_SEM_DOCTOR_STATS_MUTEX, sems->doctor_stats_mutex);
    semaphore_destroy(STR_SEM_RESULTS_MUTEX, sems->results_mutex);
    semaphore_destroy(STR_SEM_TERMINATE_MUTEX, sems->terminate_mutex);
    
}
