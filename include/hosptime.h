// Grupo 013
// 60522 - João Rebelo
// 60807 - Juliana Duarte
// 59830 - Larissa Oliveira

#include "memory.h"

/* Função que regista o tempo de criação de uma admissão
*/
void register_create_time(struct admission* ad);

/* Função que regista o tempo de receção de uma admissão por um paciente.
*/
void register_patient_time(struct admission* ad);

/* Função que regista o tempo de processamento de uma admissão por um rececionista
*/
void register_receptionist_time(struct admission* ad);

/* Função que regista o tempo de processamento de uma admissão por um médico.
*/
void register_doctor_time(struct admission* ad);
