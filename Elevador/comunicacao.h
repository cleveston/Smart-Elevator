
#ifndef COMUNICACAO_H
#define	COMUNICACAO_H

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

//Socket para conexões
extern int sockfd;

//Porta para recebimento das conexões
extern int porta;

extern struct sockaddr_in serv_addr, cli_addr;
extern socklen_t clilen;

//Abre comunicação com os andares
void abreComunicacao();

#endif

