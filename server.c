//*****Inclui as bibliotecas*****
#include<stdio.h>
#include<winsock2.h>
#include<winsock.h>
#include <sys/types.h>
#include <windows.h>
#include<string.h>
#include<locale.h>

#define SERVER_PORT 10222
#define MAX_BUFFER 1024

typedef struct{
    char file_name[50];
    char port[6];
    char ip[16];
}struct_cliente;

int aponta_arquivo();

int main(){
    int arquivo_lido, permissao, status;
    setlocale(LC_ALL, "Portuguese");
    // Inicializa o uso da DLL Winsock por um processo.
    WSADATA dado;
    SOCKET sock_servidor;
    // Inicia comunicação com usuário.
    printf("Rastrear arquivo?\n 1 - sim\n");
    scanf("%d", &permissao);
    if(permissao == 1){
        // Função para coletar informações do arquivo.
        aponta_arquivo();
    }

    struct sockaddr_in addr_servidor, addr_cliente;
	int addr_len = sizeof(SOCKADDR);
    char *buffer;
    buffer = (char*)malloc(MAX_BUFFER);
    FILE *fPtr;
    fPtr = fopen("repositorio", "rb");
    fread(&arquivo_lido, sizeof(arquivo_lido), 1, fPtr);
    char nome_arquivo[50];
    struct_cliente dados_repositorio[arquivo_lido], dados_arq;
    fread(dados_repositorio, sizeof(dados_repositorio), arquivo_lido, fPtr);
    fclose(fPtr);

    // Trabalhando com sockets no windows.
    if(WSAStartup(MAKEWORD(2, 2), &dado)!= NO_ERROR){
        printf("Falha na função WSAStartup!\n");
        return -1;
    }

    // Instanciando o socket.
    sock_servidor=socket(AF_INET,SOCK_DGRAM,0);
    if(sock_servidor<0){
        printf("O socket não foi criado!\n");
        return -1;
    }

    memset(&addr_servidor, 0, sizeof(addr_servidor));
    addr_servidor.sin_family = AF_INET;
    addr_servidor.sin_port = htons(SERVER_PORT);
    addr_servidor.sin_addr.s_addr = htonl(INADDR_ANY);

    // Associa o socket a uma porta.
    if(bind(sock_servidor,(struct sockaddr *)&addr_servidor , sizeof(addr_servidor)) < 0){ // Retorna um valor negativo em caso de erro.
        printf("Erro na função bind() : %i\n", WSAGetLastError()); // Função chamada caso WSAStartup falhe.
        return -1;
    }
    // Fila de conexões pendentes.
    listen(sock_servidor, 1); // Numero de conexões permitidas na fila de entrada = 1.
    printf("Espera a conexao de um cliente!\n");
    while(1){
        memset(buffer,'\0', MAX_BUFFER);
        if ((recvfrom(sock_servidor, buffer, MAX_BUFFER, 0, (struct sockaddr *) &addr_cliente, &addr_len)) == SOCKET_ERROR){
            printf("Erro da função recvfrom(): %d\n" , WSAGetLastError());
            exit(1);
        }
        if(buffer[0]!='\0'){
            strcpy(nome_arquivo, buffer);
            status=0;
			int i;
            for(i=0; i<arquivo_lido; i++){
                if(strcmp(dados_repositorio[i].file_name, nome_arquivo)==0){
                    dados_arq=dados_repositorio[i];
                    printf("Arquivo %s apontado!\n", nome_arquivo);
                    status = 1;
                    break;
                }
            }
            memset(buffer,'\0', MAX_BUFFER);
            if(status == 1){
                buffer[0]='1';
                strcat(buffer,dados_arq.port);
                if (sendto(sock_servidor, buffer, strlen(buffer) , 0 , (struct sockaddr *) &addr_cliente, sizeof(addr_cliente)) == SOCKET_ERROR){
                    printf("Erro da função sendto() : %d\n" , WSAGetLastError());
                    exit(1);
                }
                printf("Dados do arquivo enviado.\n Aguardando dados\n");
            }
            else if (status == 0){
                strcpy(buffer, "0connect: no such file or directory");
                if (sendto(sock_servidor, buffer, strlen(buffer) , 0 , (struct sockaddr *) &addr_cliente, sizeof(addr_cliente)) == SOCKET_ERROR){
                    printf("Erro da função sendto() : %d\n" , WSAGetLastError());
                    exit(1);
                }
                printf("Arquivo não localizado.\n Aguardando dados\n");
            }
        }

    }


}

int aponta_arquivo(){
    FILE *fPtr;
    int qnt_arquivo=0;
    fPtr=fopen("repositorio", "rb");
    fread(&qnt_arquivo, sizeof(int), 1, fPtr);
    struct_cliente inserir, existente[qnt_arquivo];
    fread(&existente, sizeof(struct_cliente), qnt_arquivo,  fPtr);
    fclose(fPtr);
    printf("Insira o nome do arquivo!\n");
    scanf("%s", inserir.file_name);
    fflush(stdin);
    printf("Insira a porta do cliente!\n");
    scanf("%s", inserir.port);
    fflush(stdin);
    strcpy(inserir.ip, "127.0.0.1");
    qnt_arquivo++;
    fPtr=fopen("repositorio", "wb");
    fwrite(&qnt_arquivo, sizeof(int), 1, fPtr);
    fwrite(&existente, sizeof(struct_cliente), qnt_arquivo-1, fPtr);
    fwrite(&inserir, sizeof(struct_cliente), 1, fPtr);
    fclose(fPtr);
    printf("Os dados do arquivo foram informados!\n");
    return 0;
}
