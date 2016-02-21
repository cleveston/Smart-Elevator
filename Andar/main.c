//-------------------------------------------------------------------------//
//                  Universidade Federal de Santa Maria                    //
//                   Curso de Engenharia de Computação                     //
//                  Sistemas Operacionais de Tempo Real                    //
//         								   //
//								 	   //
//   Autor: Iury Cleveston (201220748)                                     //
//   		                                                           //
//   Data: 15/06/2015                                                      //
//=========================================================================//
//                         Descrição do Programa                           //
//=========================================================================//
//   Elevador Inteligente - Andar                                          //
//                                                                         //
//-------------------------------------------------------------------------//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <pthread.h>

#define TAMANHO_BUFFER 256

//Definição do estado em que a porta pode ficar
#define ABERTA 1
#define FECHADA 0

//Protótipos
void *recebeComando();
void *enviaComando();
void rotinaDesativacao();
void obtemHora(char* buffer);

//Socket para leitura
int sockfd;

//Definição de qual é o andar
int andar = 0;

//Definição da porta do elevador
int porta = FECHADA;

//Threads para lidar com o andar
pthread_t thread_recebe, thread_envia;

void main(int argc, char *argv[]) {
    int portno;
    struct sockaddr_in serv_addr;

    int sig;

    sigset_t s;

    if (argc < 3) {
        printf("Uso: %s nomehost porta\n", argv[0]);
        exit(1);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        printf("Erro criando socket!\n");
        exit(1);
    }

    bzero((char *) &serv_addr, sizeof (serv_addr));

    serv_addr.sin_family = AF_INET;

    inet_aton(argv[1], &serv_addr.sin_addr);

    serv_addr.sin_port = htons(portno);

    //Conecta-se ao elevador
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0) {
        printf("Erro conectando!\n");
        exit(1);
    }

    printf("-------------------------------------\n");
    printf("Conectado ao Elevador!\n");

    //Thread que irá receber comandos do elevador
    pthread_create(&thread_recebe, NULL, recebeComando, NULL);

    //Thread que irá enviar comandos para o elevador
    pthread_create(&thread_envia, NULL, enviaComando, NULL);

    //Esvazia set
    sigemptyset(&s);

    //Adiciona Sinal de Crtl-C
    sigaddset(&s, SIGINT);

    sigprocmask(SIG_BLOCK, &s, NULL);

    while (1) {

        //Aguarda Sinal
        sigwait(&s, &sig);

        //Executa rotina de desativação
        rotinaDesativacao();

    }

}

//Função que recebe comandos do elevador

void *recebeComando() {

    char buffer[TAMANHO_BUFFER];
    char comando[TAMANHO_BUFFER];
    int n;

    while (1) {
        //Zera buffer
        bzero(comando, sizeof (comando));

        //Recebe dados do elevador
        n = recv(sockfd, comando, TAMANHO_BUFFER, 0);

        obtemHora(buffer);

        //Se houve erro
        if (n <= 0) {
            printf("\n[%s] Erro lendo do socket!\n", buffer);
            //Executa rotina de finalização
            rotinaDesativacao();

        } else if (strncmp(comando, "andar", 2) == 0) {

            char lixo[TAMANHO_BUFFER];

            //Obtem o andar que está sendo passado pelo elevador
            sscanf(comando, "%s %d", lixo, &andar);

            printf("\r                                     ");
            printf("\rAndar %d Pronto!\n", andar);
            printf("-------------------------------------\n");
            printf("Digite o comando (ou ajuda):");
            fflush(stdout);

        } else if (strcmp(comando, "abrePorta") == 0) {

            //Abre porta do elevador
            porta = ABERTA;

            printf("\r                               ");
            printf("\r[%s] Elevador Chegou - Abrindo Porta!\n", buffer);
            printf("Digite o comando (ou ajuda):");
            fflush(stdout);

        } else if (strcmp(comando, "fechaPorta") == 0) {
            //Fecha porta do elevador
            porta = FECHADA;

            printf("\r                               ");
            printf("\r[%s] Elevador Saindo - Fechando Porta!\n", buffer);
            printf("Digite o comando (ou ajuda):");
            fflush(stdout);

        } else {

            printf("\r                               ");
            printf("\r[%s] %s!\n", buffer, comando);
            printf("Digite o comando (ou ajuda):");
            fflush(stdout);

        }
    }
}

//Função que envia comandos para o elevador

void *enviaComando() {

    int n;

    char comando[TAMANHO_BUFFER];

    while (1) {
        //Zera buffer
        bzero(comando, sizeof (comando));

        printf("\nDigite o comando (ou ajuda): ");

        fgets(comando, TAMANHO_BUFFER, stdin);

        //Escreve no socket
        n = send(sockfd, comando, TAMANHO_BUFFER, 0);

        //Se houve erro
        if (n <= 0) {
            printf("Erro escrevendo no socket!\n");
            //Executa rotina de desativação
            rotinaDesativacao();
        } else if (strcmp(comando, "ajuda\n") == 0) { //Se o andar pediu ajuda

            printf("----------------- AJUDA -----------------\n");
            printf("ir ANDAR\n");
            printf("emergencia\n");
            printf("ativar\n");
            printf("desativar\n");
            printf("-----------------------------------------\n");

        } else if (strcmp(comando, "desativar\n") == 0) { //Se o andar desativado

            printf("----------------- -----------------------\n");
            printf("ANDAR DESATIVADO\n");
            printf("-----------------------------------------\n");

        } else if (strcmp(comando, "ativar\n") == 0) { //Se o andar ativado

            printf("----------------- -----------------------\n");
            printf("ANDAR ATIVADO\n");
            printf("-----------------------------------------\n");
        }
    }

}

//Obtém a hora exata do sistema

void obtemHora(char* buffer) {

    volatile time_t rawtime;
    struct tm * timeinfo;

    //Obtem a hora exata e formata
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 25, "%I:%M:%S", timeinfo);

}

//Rotina de Desativação

void rotinaDesativacao() {

    printf("\n-------------------------------------\n");
    printf("Andar sendo Desativado!");
    printf("\n-------------------------------------\n");

    //Fecha socket
    close(sockfd);

    //Mata thread que recebe comandos
    pthread_kill(thread_recebe, SIGTERM);

    //Mata thread que envia comandos
    pthread_kill(thread_envia, SIGTERM);

    exit(0);
}
