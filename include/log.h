// Grupo 013
// 60522 - João Rebelo
// 60807 - Juliana Duarte
// 59830 - Larissa Oliveira

/* Função que inicia a escrita do log do hospital. 
 * Abre o ficheiro onde deve ser escrito.
 */
void setup_log(char* filename);

/* Função que escreve uma nova linha no Log, correspondente a uma operação
 * de um utilizador. 
 */
void write_to_log(char* arg);

/* Função que termina a escrita do log do hospital. 
 * Fecha o ficheiro onde deve ser escrito.
 */
void close_log();
