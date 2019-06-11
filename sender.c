//*****Inclui as bibliotecas*****
#include<stdio.h>
#include<winsock2.h>
#include<winsock.h>
#include<sys/types.h>
#include<string.h>
#include<locale.h>

#define MAX_BUFFER 1024
#define LOCAL_PORT 10333

int checksum(char *mensagem, int tam_msg);

int main(){
    setlocale(LC_ALL,"Portuguese");
    // Inicializa o uso da DLL Winsock por um processo.
    WSADATA dado;
    SOCKET meuSocket; // Cria variavel socket.
    // sockaddr_in para ser usado com IPv4.
    struct sockaddr_in addr_cliente1, addr_cliente; // Estrutura de endereçamento.
    int addr_tam = sizeof(SOCKADDR), status, cont_pac, msg_tam;
    char *mensagem, nome_arquivo[50], ch_ree, *buffer_leitura;
    mensagem = (char*) malloc(MAX_BUFFER);
    buffer_leitura = (char*) malloc(MAX_BUFFER);
    FILE *arquivo;
    // Trabalhando com sockets no windows.
    if(WSAStartup(MAKEWORD(2, 2), &dado)!= NO_ERROR){
        printf("Falha na funcao WSAStartup!\n");
        return -1;
    }
    // Instanciando o socket.
    meuSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(meuSocket < 0){
        printf("O socket não foi criado!\n");
        return -1;
    }
    memset(&addr_cliente, 0, sizeof(SOCKADDR));
    addr_cliente.sin_family = AF_INET; // Define o tipo de família do protocolo.
    // Converte um unsigned short
    addr_cliente.sin_port = htons(LOCAL_PORT); // Numero da porta a ser usada (Transformado).
    addr_cliente.sin_addr.s_addr = inet_addr("127.0.0.1"); // Endereço IP do host de destino.
    // Associa o socket a uma porta
    if(bind(meuSocket, (struct sockaddr *)&addr_cliente, sizeof(SOCKADDR)) < 0){ // Retorna um valor negativo em caso de erro.
        printf("Erro na funcao bind() : %i\n", WSAGetLastError()); // funcao chamada caso WSAStartup falhe.
        return -1;
    }
    // Fila de conexões pendentes.
  /*  if((listen(meuSocket, 1) == SOCKET_ERROR)){
        printf("Erro na funcao listen() : %i\n", WSAGetLastError());
        return -1;
    } // Numero de conexões permitidas na fila de entrada = 1.*/
    listen(meuSocket, 1);
    printf("Espera a conexao de um cliente!\n");
    ch_ree='1';
    while(1){
        memset(mensagem,'\0', MAX_BUFFER);
        if ((recvfrom(meuSocket, mensagem, MAX_BUFFER, 0, (struct sockaddr *) &addr_cliente1, &addr_tam)) == -1){
            printf("Erro da funcao recvfrom() : %d\n" , WSAGetLastError());
            exit(1);
        }

        if(mensagem[0] != '\0'){
           strcpy(nome_arquivo, mensagem);
           arquivo=fopen(nome_arquivo, "rb");
           if(arquivo == NULL){
                strcpy(mensagem, "connect: no such file or directory");
                // sendto usada para UDP
                if (sendto(meuSocket, mensagem, strlen(mensagem) , 0 , (struct sockaddr *) &addr_cliente1, sizeof(SOCKADDR)) == -1){
                    printf(" Erro da funcao sendto(): %d\n" , WSAGetLastError());
                    exit(1);
                }
           }
           else{
               strcpy(mensagem, "5");
                if (sendto(meuSocket, mensagem, strlen(mensagem) , 0 , (struct sockaddr *)&addr_cliente1, sizeof(SOCKADDR)) == -1){
                    printf("Erro da funcao sendto(): %d\n" , WSAGetLastError());
                    exit(1);
                }
                printf("Enviando arquivo %s\n", nome_arquivo);
                cont_pac=0;
                status=0;
                while(1){
                    memset(mensagem,'\0', MAX_BUFFER); // Limpa a memória.
                    msg_tam=fread(&mensagem[4], 1, MAX_BUFFER-4, arquivo); // Carrega o arquivo para ser enviado.
                    if(msg_tam == MAX_BUFFER-4){
                        mensagem[0]='1';
                        checksum(mensagem, msg_tam);
                        status = 1;
                    }
                    else{
                        mensagem[0]='0';
                        checksum(mensagem, msg_tam);
                        status=0;
                    }
                    if(ch_ree == '1'){
                        ch_ree = '0';
                    }
                    else{
                        ch_ree = '1';
                    }
                    mensagem[1] = ch_ree;
                    if (sendto(meuSocket, mensagem, msg_tam+4 , 0 , (struct sockaddr *) &addr_cliente1, sizeof(SOCKADDR)) == -1){
                        printf("Erro da funcao sendto(): %d\n" , WSAGetLastError());
                        exit(1);
                    }
                    cont_pac++;
                    // Aguardando ack
                    while(1){
                        memset(buffer_leitura, '\0', MAX_BUFFER);
                        if ((recvfrom(meuSocket, buffer_leitura, MAX_BUFFER, 0, (struct sockaddr *) &addr_cliente1, &addr_tam)) == -1){
                            printf("Erro da funcao recvfrom(): %d\n" , WSAGetLastError());

                            exit(1);
                        }
                        if(mensagem[0]!='\0'){
                            printf("mensagem = %s\n", buffer_leitura);
                            if(strcmp(buffer_leitura, "ack1")==0){
                                printf("ack %d ok\n",cont_pac);
                                break;
                            }
                            else if(strcmp(buffer_leitura, "ack0")==0){
                                    printf("ack %d erro\nReenviar pacote", cont_pac);
                                    if (sendto(meuSocket, mensagem, msg_tam+4 , 0 , (struct sockaddr *) &addr_cliente1, sizeof(SOCKADDR)) == -1){
                                        printf("Erro da funcao sendto(): %d\n" , WSAGetLastError());
                                        exit(1);
                                    }
                                    system("pause");
                            }

                        }

                    }
                    if(status==0){
                        break;
                    }
                }

           }
           printf("Arquivo enviado!\n");
           fclose(arquivo);

        }

    }
}

int checksum(char *mensagem, int tam_msg){ // Faz a soma de verificação
    unsigned int seed=0;
    int i;
    char ch[3];
    for(i=4; i<tam_msg+4; i++){
        seed+=mensagem[i];
    }
    seed=seed%16;
    if(seed<10){
        seed=seed*10;
    }
    printf("Checksum = %d\n", seed);
    itoa(seed, ch, 10); // Converte numero para string
    mensagem[2]=ch[0];
    mensagem[3]=ch[1];
    return 0;
}
