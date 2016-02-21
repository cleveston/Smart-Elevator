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
//   Elevador Inteligente - Elevador                                       //
//                                                                         //
//-------------------------------------------------------------------------//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>	
#include <bits/sigthread.h>
#include <pthread.h>

#include "periodica.h"
#include "comunicacao.h"
#include "grafo.h"

//Tamanho do buffer para transmissão
#define TAMANHO_BUFFER 256

//Definição dos estados dos andares
#define ATIVO 1
#define DESATIVADO 2
#define PROBLEMA 3

//Estrutura que guarda as informações dos andares

typedef struct {
    int andar;
    int socket;
    int estado;
    MBvertex *vertice;
    pthread_t thread;
    struct ANDAR *prox;
    struct ANDAR *ant;
} ANDAR;

//Estrutura que armazenará o prédio

typedef struct {
    ANDAR *terreo;
    ANDAR *terraco;
} PREDIO;

//Protótipos
ANDAR * criaAndar();
ANDAR* findAndar(int andar);
void addAndar(ANDAR* a);
void remover(ANDAR *a);
void imprimeAndares();
int countAndares();
void rotinaDesligamento();
void *iniciaFuncionamento();
void *trataAndar(void *structAndar);
void enviaComando(char *comando, ANDAR *a);
void conectaAndares();
void *verificaFalha();
void desloca(ANDAR *proximoAndar);
void desativarAndar(ANDAR *a);
void salvaLog(char *buffer);

//Definição do mutex do prédio
pthread_mutex_t mutexPredio;

//Definição do mutex do trajeto
pthread_mutex_t mutexTrajeto;

//Fila que representa o prédio
PREDIO *predio;

//Grafo que representa o trajeto do elevador
MBgraph5 *trajeto;

//Número de andares no prédio
int nAndares = 0;

//Número de requisicões
int nRequisicao = 0;

//Posição atual do elevador
ANDAR *andarAtual;

//Mostra o trajeto
int TRAJETO = 0;
int PESO = 1;

void main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Erro, porta não definida!\n");
        exit(1);
    }

    // Threads para lidar com o elevador
    pthread_t thread_verificaFalha;
    pthread_t thread_iniciaFuncionamento;

    //Configura timer periódico
    configuraTimer();

    //Definie porta para conexão
    porta = atoi(argv[1]);

    //Abre comunicação com os andares
    abreComunicacao();

    //Cria prédio
    predio = (PREDIO*) malloc(sizeof (PREDIO));

    //Cria trajeto
    trajeto = MBgraph5_create();

    //Inicia variáveis de mutex
    pthread_mutex_init(&mutexPredio, NULL);
    pthread_mutex_init(&mutexTrajeto, NULL);

    printf("---------------------------------------\n");
    printf("Bem Vindo ao Elevador Inteligente!");
    printf("\n---------------------------------------\n");

    //Define quantos andares o prédio terá
    printf("Defina quantos andares seu prédio tem: ");
    scanf("%d", &nAndares);

    //Função que espera todos os andares se conectarem
    conectaAndares();

    //Fecha socket, pois já temos o número suficiente de andares
    close(sockfd);

    //Cria thread para gerenciar funcionamento do elevador
    pthread_create(&thread_iniciaFuncionamento, NULL, iniciaFuncionamento, NULL);

    //Cria thread para verificar se houve algum andar desconectado
    pthread_create(&thread_verificaFalha, NULL, verificaFalha, NULL);

    //Configuração do Crtl-C
    sigset_t s;
    int sig;

    //Esvazia set
    sigemptyset(&s);

    //Adiciona Sinal de Crtl-C
    sigaddset(&s, SIGINT);

    sigprocmask(SIG_BLOCK, &s, NULL);

    //Aguarda por algum sinal de Crtl-C
    while (1) {

        //Aguarda Sinal
        sigwait(&s, &sig);

        printf("\n-------------------------------------\n");
        printf("Elevador Fechando Conexões!");
        printf("\n-------------------------------------\n");

        //Mata threads responsáveis pelo elevador
        pthread_kill(thread_verificaFalha, SIGTERM);
        pthread_kill(thread_iniciaFuncionamento, SIGTERM);

        //Executa rotina de desligamento
        rotinaDesligamento();

        free(predio);

        //Libera trajeto
        MBgraph5_delete(trajeto);

        //Destroi o mutexes
        pthread_mutex_destroy(&mutexPredio);
        pthread_mutex_destroy(&mutexTrajeto);

        exit(0);

    }

}

//Função que conecta todos os andares

void conectaAndares() {

    printf("---------------------------------------\n");
    printf("Elevador Aguardando Andares se Conectarem!");
    printf("\n---------------------------------------\n");

    //Aguarda todos os andares estarem conectados ao elevador
    while (countAndares() < nAndares) {

        int socket;

        //Aguarda o andar se conectar
        socket = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        //Caso não for possível aceitar o andar
        if (socket < 0) {
            printf("Erro na Conexão do Andar!\n");
        } else {

            //Cria novo andar		
            ANDAR *a = criaAndar();
            a->socket = socket;
            a->estado = ATIVO;

            //Adiciona andar ao prédio
            addAndar(a);

            //Cria thread para tratar o andar
            pthread_create(&(a->thread), NULL, trataAndar, (void*) a);

        }

    }

    //Inicia o elevador no térreo
    andarAtual = predio->terreo;

    printf("---------------------------------------\n");
    printf("Andares Conectados!");
    printf("\n---------------------------------------\n");
}

//Tarefa periódica que verifica se houve falha de comunicação com algum andar

void *verificaFalha() {

    //Estrutura que armazena as configurações do período
    struct periodic_info info;

    char buffer[TAMANHO_BUFFER];

    //Determina contagem de 1s
    configuraPeriodica(1, &info);

    while (1) {

        if (predio->terreo != NULL) {

            ANDAR *a = predio->terreo;

            do {

                //Se algum andar está com problema
                if (a->estado == PROBLEMA) {

                    printf("---------------------------------------\n");
                    printf("Falha no Sistema!\n");
                    printf("---------------------------------------\n");
                    printf("Elevador Aguardando Andares se Reconectarem!");
                    printf("\n---------------------------------------\n");
                    fflush(stdout);

                    //Abre comunicação com os andares
                    abreComunicacao();

                    if (predio->terreo != NULL) {

                        ANDAR *b = predio->terreo;

                        //Move elevador para o térreo
                        MBgraph5_add_edge(trajeto, andarAtual->vertice, b->vertice, 0);

                        //Desativa todos os andares
                        do {

                            desativarAndar(b);

                            b->estado = DESATIVADO;

                            b = b->prox;
                        } while (b != NULL);

                    }

                    int socket;


                    //Aguarda o andar se conectar
                    socket = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

                    //Caso não for possível aceitar o andar
                    if (socket < 0) {
                        printf("Erro na Conexão do Andar!\n");
                    } else {


                        a->socket = socket;
                        a->estado = ATIVO;

                        //Cria thread para tratar o andar
                        pthread_create(&(a->thread), NULL, trataAndar, (void*) a);

                        //Obtém a hora
                        obtemHora(buffer);
                        printf("[%s] Andar %d reconectou-se!\n", buffer, a->andar);

                        //Monta comando 
                        sprintf(buffer, "andar %d", a->andar);

                        //Envia comando para o andar
                        enviaComando(buffer, a);

                        //Fecha socket novamente
                        close(sockfd);

                        ANDAR *b = predio->terreo;

                        //Ativa todos os andares
                        do {

                            b->estado = ATIVO;

                            b = b->prox;
                        } while (b != NULL);

                        obtemHora(buffer);
                        printf("[%s] Elevador entrou em funcionamento!\n", buffer);

                    }

                }

                a = a->prox;
            } while (a != NULL);

        }

        //Aguarda 1s
        aguardaSeg(1, &info);
    }

}

//Função que gerenciará funcionamento do elevador

void *iniciaFuncionamento() {

    MBiterator *edges;
    MBedge *edge;

    char buffer[TAMANHO_BUFFER];

    obtemHora(buffer);
    printf("[%s] Elevador entrou em funcionamento!\n", buffer);

    //Sempre verifica se tem requisições
    while (1) {

        ANDAR *proximoAndar = andarAtual;
        int distancia = 0;

        //Verifica se chegou alguma requisição
        if (MBgraph5_get_edge_count(trajeto) > 0) {

            //Trava mutex
            pthread_mutex_lock(&mutexTrajeto);

            edges = MBgraph5_get_edges(trajeto);

            //Encontra próximo andar
            while ((edge = MBiterator_get(edges))) {

                int andarPartida = (int) MBvertex_get_data(MBedge_get_from(edge));

                int andarDestino = (int) MBvertex_get_data(MBedge_get_to(edge));

                int peso = (PESO == 1) ? MBedge_get_weight(edge) : 0;

                //Faz o cálculo a partir do andar atual
                if (andarPartida == andarAtual->andar && andarAtual->andar != andarDestino) {

                    //Calcula distância
                    int novadistancia = abs(andarAtual->andar - andarDestino) + peso;

                    //Atualiza valor para a menor distância
                    if (novadistancia <= distancia || distancia == 0) {
                        distancia = novadistancia;
                        proximoAndar = findAndar(andarDestino);
                    }

                }

            }

            edges = MBgraph5_get_edges(trajeto);

            //Atualiza arestas para proximo andar
            while ((edge = MBiterator_get(edges))) {

                int andarPartida = (int) MBvertex_get_data(MBedge_get_from(edge));

                int andarDestino = (int) MBvertex_get_data(MBedge_get_to(edge));

                if (TRAJETO == 1) {

                    printf("<Andar %d, Andar %d>\n", andarPartida, andarDestino);

                }

                if (andarPartida == andarAtual->andar) {

                    //Remove arresta
                    MBgraph5_remove_edge(trajeto, andarAtual->vertice, MBedge_get_to(edge));

                    if (andarDestino != proximoAndar->andar) {
                        //Adiciona arresta
                        MBgraph5_add_edge(trajeto, proximoAndar->vertice, MBedge_get_to(edge), MBedge_get_weight(edge));

                        //Requisição completada
                        nRequisicao--;

                    }

                    //Atualiza array
                    edges = MBgraph5_get_edges(trajeto);
                }

            }

            MBiterator_delete(edges);

            //Libera mutex
            pthread_mutex_unlock(&mutexTrajeto);

            if (proximoAndar->andar != andarAtual->andar) {
                //Desloca elevador para andar informado

                desloca(proximoAndar);
            }
        }

    }

}

//Desloca elevador para andar informado

void desloca(ANDAR * proximoAndar) {

    //Estrutura que armazena as configurações do período
    struct periodic_info info;

    //Determina contagem de 1s
    configuraPeriodica(1, &info);

    char buffer[TAMANHO_BUFFER];

    obtemHora(buffer);
    printf("[%s] Deslocando para Andar %d!\n", buffer, proximoAndar->andar);

    //Trava mutex
    pthread_mutex_lock(&mutexTrajeto);

    //Envia comando
    enviaComando("fechaPorta", andarAtual);

    //Percorre até chegar no andar correto
    while (andarAtual->andar != proximoAndar->andar) {

        //Aguarda 1 segundo
        aguardaSeg(2, &info);

        if (andarAtual->andar > proximoAndar->andar)
            andarAtual = andarAtual->ant;

        else
            andarAtual = andarAtual->prox;

    }

    //Libera mutex
    pthread_mutex_unlock(&mutexTrajeto);

    obtemHora(buffer);
    printf("[%s] Chegou no Andar %d!\n", buffer, proximoAndar->andar);

    //Envia comando
    enviaComando("abrePorta", andarAtual);

    //Aguarda 3 segundos
    aguardaSeg(3, &info);

}

//Cria andar

ANDAR * criaAndar() {

    char nome[TAMANHO_BUFFER];

    //Trava mutex
    pthread_mutex_lock(&mutexPredio);

    ANDAR *a = (ANDAR*) malloc(sizeof (ANDAR));

    a->andar = countAndares();

    sprintf(nome, "Andar %d", a->andar);

    a->vertice = MBgraph5_add(trajeto, nome, (void*) a->andar);
    a->socket = 0;
    a->thread = 0;
    a->ant = NULL;
    a->prox = NULL;

    //Libera mutex
    pthread_mutex_unlock(&mutexPredio);

    return a;
}

//Adiciona andar ao prédio

void addAndar(ANDAR * a) {

    char comando[TAMANHO_BUFFER];
    char buffer[TAMANHO_BUFFER];

    //Trava mutex
    pthread_mutex_lock(&mutexPredio);

    //Verifica se existe o térreo
    if (predio->terreo != NULL) {

        ANDAR *ant = predio->terraco;
        a->ant = ant;
        ant->prox = a;

    } else {

        //Atualiza térreo do prédio

        predio->terreo = a;
    }

    //Atualiza terraço do prédio
    predio->terraco = a;

    //Libera mutex
    pthread_mutex_unlock(&mutexPredio);

    //Obtém a hora
    obtemHora(buffer);
    printf("[%s] Andar %d pronto!\n", buffer, a->andar);

    //Monta comando 
    sprintf(comando, "andar %d", a->andar);

    //Envia comando para o andar
    enviaComando(comando, a);

}

//Remove andar do prédio

void remover(ANDAR * a) {

    //Trava mutex
    pthread_mutex_lock(&mutexPredio);

    ANDAR *prox = a->prox;
    ANDAR *ant = a->ant;

    if (ant != NULL)
        ant->prox = prox;
    if (prox != NULL)
        prox->ant = ant;

    //Se o andar a ser retirado for o térreo, atualiza prédio.
    if (a == predio->terreo) {
        if (a == a->prox)
            predio->terreo = NULL;
        else
            predio->terreo = a->prox;

    }

    //Se o andar a ser retirado for o terraço, atualiza prédio
    if (a == predio->terraco) {

        predio->terraco = ant;
        ant->prox = NULL;
    }

    //Rotina para desativar o andar
    desativarAndar(a);

    //Libera memória
    free(a);

    //Libera mutex
    pthread_mutex_unlock(&mutexPredio);

}

//Imprime os andares

void imprimeAndares() {

    if (predio->terreo != NULL) {

        ANDAR *a = predio->terreo;

        printf("\nAndares em Funcionamento\n");
        printf("-------------------------------------\n");

        do {
            printf("Andar %d\n", a->andar);

            a = a->prox;
        } while (a != NULL);
    }

}

//Conta andares do prédio

int countAndares() {

    int andares = 0;

    if (predio->terreo != NULL) {

        ANDAR *a = predio->terreo;

        do {
            andares++;

            a = a->prox;
        } while (a != NULL);
    }

    return andares;

}

//Encontra struct do andar pelo numero

ANDAR * findAndar(int andar) {

    ANDAR *a = NULL;

    if (predio->terreo != NULL) {

        a = predio->terreo;

        do {
            if (a->andar == andar) {
                break;
            }

            a = a->prox;
        } while (a != NULL);
    }

    return a;

}


//Rotina de Desligamento

void rotinaDesligamento() {

    //Trava mutex
    pthread_mutex_lock(&mutexPredio);

    ANDAR* aux = predio->terreo;

    //Percorre toda o predio
    while (aux != NULL) {

        ANDAR* tmp = aux;
        aux = aux->prox;

        //Fecha socket
        close(tmp->socket);

        //Finaliza a thread
        pthread_exit(tmp->thread);

        //Remove vértice do trajeto
        MBgraph5_remove(trajeto, tmp->vertice);

        free(tmp);
    }

    predio->terreo = NULL;

    //Libera mutex
    pthread_mutex_unlock(&mutexPredio);
}

//Função que gerencia cada andar

void *trataAndar(void *structAndar) {

    ANDAR *a = (ANDAR*) structAndar;

    int n;

    char comando[TAMANHO_BUFFER];
    char buffer[TAMANHO_BUFFER];

    while (1) {

        //Zera buffer
        bzero(comando, sizeof (comando));

        //Lê socket
        n = read(a->socket, comando, TAMANHO_BUFFER);

        //Obtém a hora
        obtemHora(buffer);

        //Verifica se a leitura ocorreu com sucesso
        if (n <= 0) {
            printf("[%s] Andar %d com problema!\n", buffer, a->andar);

            //Fechar conexão
            close(a->socket);

            //Desativa andar
            desativarAndar(a);

            a->estado = PROBLEMA;

            pthread_exit(NULL);

        } else if (strcmp(comando, "desativar\n") == 0) { //Se andar foi desativado

            printf("[%s] Andar %d foi desativado\n", buffer, a->andar);

            //Rotina para desativar o andar
            desativarAndar(a);

            //Desativa andar
            a->estado = DESATIVADO;

        } else if (strcmp(comando, "ativar\n") == 0) { //Se andar foi ativado

            printf("[%s] Andar %d foi ativado\n", buffer, a->andar);

            //Ativar andar
            a->estado = ATIVO;

        } else if (strcmp(comando, "trajeto\n") == 0) {

            if (TRAJETO == 1) {
                //Mostra trajeto
                TRAJETO = 0;
            } else {
                TRAJETO = 1;
            }

        } else if (strcmp(comando, "peso\n") == 0) {

            if (PESO == 1) {
                //Adiciona peso ao trajeto
                PESO = 0;
            } else {
                PESO = 1;
            }

        } else if (strcmp(comando, "emergencia\n") == 0) {

            //Verifica se o andar está em funcionamento
            if (a->estado == ATIVO) {

                printf("[%s] Emergência Andar %d\n", buffer, a->andar);

                //Trava mutex
                pthread_mutex_lock(&mutexTrajeto);

                //Adiciona andar no trajeto
                MBgraph5_add_edge(trajeto, a->vertice, predio->terreo->vertice, -5);

                //Não adiciona aresta, se o andar atual for o mesmo da partida
                if (a->andar != andarAtual->andar) {
                    //Adiciona andar atual ao andar do passageiro
                    MBgraph5_add_edge(trajeto, andarAtual->vertice, a->vertice, -5);
                }

                //Libera mutex
                pthread_mutex_unlock(&mutexTrajeto);

            } else {
                //Envia comando para o andar
                enviaComando("Andar em manutenção", a);

            }

        } else if (strncmp(comando, "ir", 2) == 0) {

            int andar;

            char lixo[TAMANHO_BUFFER];

            //Obtem o andar que está sendo passado pelo elevador
            sscanf(comando, "%s %d", lixo, &andar);

            //Verifica se o andar informado está fora da faixa permitida
            if (andar > countAndares() - 1) {

                //Monta comando 
                sprintf(comando, "Andar %d fora do Intervalo", andar);

                //Envia comando para o andar
                enviaComando(comando, a);

            } else if (andar == a->andar) {

                //Envia comando para o andar
                enviaComando("Você já está nesse andar", a);

            } else {

                //Encontra referência do andar informado

                ANDAR *destino = findAndar(andar);

                //Verifica se os andares estão em funcionamento
                if (destino != NULL && destino->estado == ATIVO && a->estado == ATIVO) {

                    //Trava mutex
                    pthread_mutex_lock(&mutexTrajeto);

                    //Adiciona andar no trajeto
                    MBgraph5_add_edge(trajeto, a->vertice, destino->vertice, nRequisicao);

                    //Não adiciona aresta, se o andar atual for o mesmo da partida
                    if (a->andar != andarAtual->andar) {
                        //Adiciona andar atual ao andar do passageiro
                        MBgraph5_add_edge(trajeto, andarAtual->vertice, a->vertice, nRequisicao);
                    }

                    //Incrementa numero de requiiscao
                    nRequisicao++;

                    //Libera mutex
                    pthread_mutex_unlock(&mutexTrajeto);

                    //Monta saída
                    sprintf(comando, "[%s] Requisição: Andar %d -> Andar %d\n", buffer, a->andar, destino->andar);

                    //Imprime saída na tela
                    printf(comando);

                    //Salva log da requisição
                    salvaLog(comando);


                } else {

                    //Monta comando 

                    sprintf(comando, "Andar %d em manutenção", andar);

                    //Envia comando para o andar
                    enviaComando(comando, a);

                }

            }
        }

    }

}

//Rotina para desativar um andar

void desativarAndar(ANDAR * a) {

    MBiterator *edges;
    MBedge *edge;

    //Trava mutex
    pthread_mutex_lock(&mutexTrajeto);

    //Remove andar do trajeto
    edges = MBgraph5_get_edges(trajeto);

    //Remove conexões com o andar
    while ((edge = MBiterator_get(edges))) {

        int andarPartida = (int) MBvertex_get_data(MBedge_get_from(edge));

        int andarDestino = (int) MBvertex_get_data(MBedge_get_to(edge));

        //Verifica a direção da conexão
        if (andarPartida == a->andar) {

            //Remove arresta
            MBgraph5_remove_edge(trajeto, a->vertice, MBedge_get_to(edge));

        } else if (andarDestino == a->andar) {

            //Remove arresta

            MBgraph5_remove_edge(trajeto, MBedge_get_to(edge), a->vertice);

        }

    }
    //Libera mutex
    pthread_mutex_unlock(&mutexTrajeto);

}

//Enviar comando para andar

void enviaComando(char *comando, ANDAR * a) {

    int n;

    //Escreve no socket
    n = write(a->socket, comando, TAMANHO_BUFFER);

    if (n < 0) {

        printf("Erro escrevendo no socket!\n");
    }

}

//Salva log das requisições

void salvaLog(char *buffer) {

    FILE *file = fopen("log.txt", "a+");

    fprintf(file, "%s", buffer);

    fclose(file);
}
