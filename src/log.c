// Grupo 013
// 60522 - João Rebelo
// 60807 - Juliana Duarte
// 59830 - Larissa Oliveira

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

FILE *logFile;
struct timespec logTime;
struct tm *now;
long miliseconds;
char formattedTime[25];

/* Função que inicia a escrita do log do hospital. 
 * Abre o ficheiro onde deve ser escrito.
 */
void setup_log(char* filename){
    logFile = fopen(filename, "a");
}

/* Função que escreve uma nova linha no Log, correspondente a uma operação
 * de um utilizador. 
 */
void write_to_log(char* arg){
    if( clock_gettime(CLOCK_REALTIME, &logTime) == -1) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }
    miliseconds = logTime.tv_nsec / 1000000; //nanosegundos desde tv_sec (é um delta)
    now = localtime(&logTime.tv_sec);
    strftime(formattedTime, 25, "%d/%m/%Y_%X", now);
    fprintf(logFile,"%s.%03ld %s\n",formattedTime, miliseconds, arg);
}

/* Função que termina a escrita do log do hospital. 
 * Fecha o ficheiro onde deve ser escrito.
 */
void close_log(){
    if( clock_gettime(CLOCK_REALTIME, &logTime) == -1) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }
    miliseconds = logTime.tv_nsec / 1000000; //nanosegundos desde tv_sec (é um delta)
    now = localtime(&logTime.tv_sec);
    strftime(formattedTime, 25, "%d/%m/%Y_%X", now);
    fprintf(logFile,"%s.%03ld end\n",formattedTime, miliseconds);
    fclose(logFile);
}