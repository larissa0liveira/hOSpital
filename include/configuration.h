// Grupo 013
// 60522 - João Rebelo
// 60807 - Juliana Duarte
// 59830 - Larissa Oliveira

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <stdio.h>
#include "main.h"

/* Função que lê os argumentos da aplicação no ficheiro dado.
 * Guarda esta informação nos campos apropriados da
 * estrutura data_container.
 */ 
void read_configuration(struct data_container *data, const char* filename);

#endif  
