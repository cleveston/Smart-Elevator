#include "comunicacao.h"

//Socket para conexões
int sockfd;

//Porta para recebimento das conexões
int porta;

struct sockaddr_in serv_addr, cli_addr;
socklen_t clilen;

//Abre comunicação com os andares

void abreComunicacao() {

    //Abre socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        printf("Erro abrindo o socket!\n");
        exit(1);
    }

    int true = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &true, sizeof (int));

    bzero((char *) &serv_addr, sizeof (serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(porta);

    //Faz o bind na porta
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0) {
        printf("Erro fazendo bind!\n");
        exit(1);
    }

    //Escuta o socket
    listen(sockfd, 5);

}
