// Grupo 013
// 60522 - João Rebelo
// 60807 - Juliana Duarte
// 59830 - Larissa Oliveira

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <unistd.h>
#include "../include/memory.h"

// NOTA: os buffers circulares como estão implementados não permitem a permanencia de admissões "inválidas".
// Ou seja, quando, por exemplo, uma admissão entra no main_patient_buffer pedindo consulta para um paciente não existente,
// a posição [out] torna-se impossível de ler (por um processo patient) e o buffer "entope". -> Este mecanismo ocorre também
// em hOSpital_profs, segundo nos pareceu.


/* Função que reserva uma zona de memória partilhada com tamanho indicado
 * por size e nome name, preenche essa zona de memória com o valor 0, e
 * retorna um apontador para a mesma. Pode concatenar o resultado da função
 * getuid() a name, para tornar o nome único para o processo.
 */
void *create_shared_memory(char *name, int size) {
    int *ptr;
    int ret;

    //concatena id ao nome
    char id[MAX_RESULTS]; 
    sprintf(id, "/%d", getpid());
    strcat(id, name);
    
    int fd = shm_open(id, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Failed to open shared memory space");
        exit(1);
    }

    ret = ftruncate(fd, size);
    if (ret == -1) {
        perror("Failed to reserve required memory space");
        exit(2);
    }

    ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    if (ptr == MAP_FAILED) {
        perror("Failed to return the pointer to the reserved memory space");
        exit(3);
    }

    return ptr;
}

/* Função que reserva uma zona de memória dinâmica com tamanho indicado
 * por size, preenche essa zona de memória com o valor 0, e retorna um
 * apontador para a mesma.
 */
void *allocate_dynamic_memory(int size)
{
    void *pointer = malloc(size);
    if(pointer == NULL) {
        perror("falhou alocação de memória dinamica");
        exit(1);
    }
    return memset(pointer, 0, size);
}

/* Função que liberta uma zona de memória partilhada previamente reservada.
 */
void destroy_shared_memory(char *name, void *ptr, int size)
{
    int ret;
    char id[MAX_RESULTS]; 
    sprintf(id, "/%d", getpid());
    strcat(id, name);


    ret = munmap(ptr, size);

    if (ret == -1) {
        perror("Unable to unmap shared memory");
    }

    ret = shm_unlink(id);
    if (ret == -1) {
        perror("Unable to unlink shared memory");
    }
    
}
/* Função que liberta uma zona de memória dinâmica previamente reservada.
 */
void deallocate_dynamic_memory(void *ptr) {
    free(ptr);
}

/* Função que escreve uma admissão no buffer de memória partilhada entre a Main
 * e os pacientes. A admissão deve ser escrita numa posição livre do buffer,
 * tendo em conta o tipo de buffer e as regras de escrita em buffers desse tipo.
 * Se não houver nenhuma posição livre, não escreve nada.
 */
void write_main_patient_buffer(struct circular_buffer *buffer, int buffer_size, struct admission *ad) {
    int in = buffer->ptrs->in;
    int out = buffer->ptrs->out;

    if((in + 1) % buffer_size != out){
        buffer->buffer[in] = *ad;
        buffer->ptrs->in = (in + 1) % buffer_size;
    }
}

/* Função que escreve uma admissão no buffer de memória partilhada entre os pacientes
 * e os rececionistas. A admissão deve ser escrita numa posição livre do buffer,
 * tendo em conta o tipo de buffer e as regras de escrita em buffers desse tipo.
 * Se não houver nenhuma posição livre, não escreve nada.
 */
void write_patient_receptionist_buffer(struct rnd_access_buffer *buffer, int buffer_size, struct admission *ad) {
    for(int i = 0; i < buffer_size; i++) {
        if(buffer->ptrs[i] == 0) {
            buffer->buffer[i] = *ad;
            buffer->ptrs[i] = 1;
            break;
        }
    }
}

/* Função que escreve uma admissão no buffer de memória partilhada entre os rececionistas
 * e os médicos. A admissão deve ser escrita numa posição livre do buffer,
 * tendo em conta o tipo de buffer e as regras de escrita em buffers desse tipo.
 * Se não houver nenhuma posição livre, não escreve nada.
 */
void write_receptionist_doctor_buffer(struct circular_buffer *buffer, int buffer_size, struct admission *ad) {
    int in = buffer->ptrs->in;
    int out = buffer->ptrs->out;

    if((in + 1) % buffer_size != out){ //se não estiver cheio
        buffer->buffer[in] = *ad;
        buffer->ptrs->in = (in + 1) % buffer_size;
    }
}

/* Função que lê uma admissão do buffer de memória partilhada entre a Main
 * e os pacientes, se houver alguma disponível para ler que seja direcionada ao paciente especificado.
 * A leitura deve ser feita tendo em conta o tipo de buffer e as regras de leitura em buffers desse tipo.
 * Se não houver nenhuma admissão disponível, afeta ad->id com o valor -1.
 */
void read_main_patient_buffer(struct circular_buffer *buffer, int patient_id, int buffer_size, struct admission *ad) {
    int in = buffer->ptrs->in;
    int out = buffer->ptrs->out;
    int ad_found = 0;

    if(in != out) { //se nao estiver vazio    
        if(buffer->buffer[out].requesting_patient == patient_id) {
            *ad = buffer->buffer[out];
            buffer->ptrs->out = (out + 1) % buffer_size;
            ad_found = 1;
        }
    }

    if(!ad_found)
        ad->id = -1;

}

/* Função que lê uma admissão do buffer de memória partilhada entre os pacientes e rececionistas,
 * se houver alguma disponível para ler (qualquer rececionista pode ler qualquer admissão).
 * A leitura deve ser feita tendo em conta o tipo de buffer e as regras de leitura em buffers desse tipo.
 * Se não houver nenhuma admissão disponível, afeta ad->id com o valor -1.
 */
void read_patient_receptionist_buffer(struct rnd_access_buffer *buffer, int buffer_size, struct admission *ad) {
    int ad_found = 0;

    for(int i = 0; i < buffer_size; i++) {
        if(buffer->ptrs[i] == 1) {
            *ad = buffer->buffer[i];
            buffer->ptrs[i] = 0;
            ad_found = 1;
            break;
        }
    }
    if (!ad_found)
        ad->id = -1;
}

/* Função que lê uma admissão do buffer de memória partilhada entre os rececionistas e os médicos,
 * se houver alguma disponível para ler dirigida ao médico especificado. A leitura deve
 * ser feita tendo em conta o tipo de buffer e as regras de leitura em buffers desse tipo. Se não houver
 * nenhuma admissão disponível, afeta ad->id com o valor -1.
 */
void read_receptionist_doctor_buffer(struct circular_buffer *buffer, int doctor_id, int buffer_size, struct admission *ad) {
    int in = buffer->ptrs->in;
    int out = buffer->ptrs->out;
    int ad_found = 0;

    if(in != out) { //se nao estiver vazio
        if(buffer->buffer[out].requested_doctor == doctor_id) {
            *ad = buffer->buffer[out];
            buffer->ptrs->out = (out + 1) % buffer_size;
            ad_found = 1;
        }
    }
    if(!ad_found)
        ad->id = -1;
}
