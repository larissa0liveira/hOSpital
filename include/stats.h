// Grupo 013
// 60522 - João Rebelo
// 60807 - Juliana Duarte
// 59830 - Larissa Oliveira

/* Função que escreve as estatísticas finais dos pacientes. 
 */
void patients_stats(struct data_container *data);

/* Função que escreve as estatísticas finais dos rececionistas. 
 */
void receptionists_stats(struct data_container *data);

/* Função que escreve as estatísticas finais dos médicos. 
 */
void doctors_stats(struct data_container *data);

/* Função que escreve o tempo dado formatado. 
 */
char* formatted_time(struct timespec *time);

/* Função que calcula o tempo total entre dois tempos
 * escreve esse tempo formatado. 
 */
char* total_adm_time(struct timespec *start, struct timespec *end);

/* Função que escreve as estatísticas finais das admissões. 
 */
void admissions_stats(struct data_container *data);

/* Função que escreve as estatisticas finais do hOSpital,
 * incluindo as estatísticas dos vários processos e das admissões. 
 */
void write_stats_to_file(struct data_container *data);