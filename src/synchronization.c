// Grupo 013
// 60522 - João Rebelo
// 60807 - Juliana Duarte
// 59830 - Larissa Oliveira

#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <fcntl.h>
#include "../include/synchronization.h"


/* Função que cria um novo semáforo com nome name e valor inicial igual a
* value. Pode concatenar o resultado da função getpid() a name, para tornar
* o nome único para o processo.
*/
sem_t * semaphore_create(char* name, int value){

	char unique_name[256];
    snprintf(unique_name, sizeof(unique_name), "%s_%d", name, getpid());

	sem_t *sem;
	sem = sem_open(unique_name, O_CREAT, 0xFFFFFFFF, value);
	if (sem == SEM_FAILED){
		perror(unique_name);
		exit(6);
	}
	return sem;
}

/* Função que destroi o semáforo passado em argumento.
*/
void semaphore_destroy(char* name, sem_t* semaphore){
	char unique_name[256];
    snprintf(unique_name, sizeof(unique_name), "%s_%d", name, getpid());

	if (sem_close(semaphore) == -1){
		perror(unique_name);
	}
	if (sem_unlink(unique_name) == -1){
		perror(unique_name);
	}
}

/* Função que faz wait a um semáforo.
*/
void semaphore_lock(sem_t* sem){
	if(sem_wait(sem) == -1) {
		perror("sem wait");
	
	}
}

/* Função que faz post a um semáforo.
*/
void semaphore_unlock(sem_t* sem){
	if(sem_post(sem) == -1) {
		perror("sem post");
		
	}
}

/* Função que inicia o processo de produzir, fazendo sem_wait nos semáforos
* corretos da estrutura passada em argumento.
*/
void produce_begin(struct prodcons* pc){
	semaphore_lock(pc->empty); //existe alguma slot vazio?
	semaphore_lock(pc->mutex); //o processo quer aceder ao buffer circular – secção
}

/* Função que termina o processo de produzir, fazendo sem_post nos semáforos
* corretos da estrutura passada em argumento.
*/
void produce_end(struct prodcons* pc){
	semaphore_unlock(pc->mutex); // o processo sai da seção crítica
	semaphore_unlock(pc->full);
}

/* Função que inicia o processo de consumir, fazendo sem_wait nos semáforos
* corretos da estrutura passada em argumento.
*/
void consume_begin(struct prodcons* pc){
	semaphore_lock(pc->full);
	semaphore_lock(pc->mutex);
}

/* Função que termina o processo de consumir, fazendo sem_post nos semáforos
* corretos da estrutura passada em argumento.
*/
void consume_end(struct prodcons* pc){
	semaphore_unlock(pc->mutex);
	semaphore_unlock(pc->empty);
}