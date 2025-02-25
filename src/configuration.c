// Grupo 013
// 60522 - João Rebelo
// 60807 - Juliana Duarte
// 59830 - Larissa Oliveira

#include <stdio.h>
#include "configuration.h"

/* Função que lê os argumentos da aplicação no ficheiro dado.
 * Guarda esta informação nos campos apropriados da
 * estrutura data_container.
 */
void read_configuration(struct data_container *data, const char* filename) {

    FILE* file = fopen(filename, "r"); //r – Abre para leitura. O ficheiro tem de existir.
    if (file == NULL) {
        perror("Erro ao tentar abrir o ficheiro");
    }
    
    while (!feof(file)){
    fscanf(file, "%d", &data->max_ads);
    fscanf(file, "%d", &data->buffers_size);
    fscanf(file, "%d", &data->n_patients);
    fscanf(file, "%d", &data->n_receptionists);
    fscanf(file, "%d", &data->n_doctors);
    fscanf(file, "%255s", data->log_filename);
    fscanf(file, "%255s", data->statistics_filename);
    fscanf(file, "%d", &data->alarm_time);
    }

    fclose(file);
}
