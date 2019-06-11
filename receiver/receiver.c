/**
 * =================================================================================
 *                        FUNCIONAMENTO TESTADO NO WINDOWS 10
 * =================================================================================
 * 
 * COM 240 - REDES DE COMPUTADORES
 * Para compilar:
 * gcc -o receiverUDP receiver.c -lws2_32 -lwsock32 -L $MingGW\lib
 */
#include <stdio.h>
#include <winsock2.h>
#include <sys/types.h>
#include <string.h>
#include <locale.h>
#define tam_buffer 1024
#define porta_servidor 10222

/* Definindo funções */
int checksum(char *buffer, int tam_msg, char *check);
int winSockInit(WSADATA *data, SOCKET *sock_cliente);
int startSockWin(WSADATA *data);
int createLocalSock(SOCKET *sock_cliente);
/* Fim */

/*
 * ======================  GERAIS ======================
 * - WSADATA -> Guarda informações referentes a implementação de um WinSocket
 * - WSAGetLastError -> Retornará o último erro enviado pelo WinSocket que está sendo executado
 * - WSAStartup -> Inicializa o uso de uma DLL específica, necessária para funcionamento do WinSock
 * - MAKEWORD -> Necessário para funcionamento do WinSock, cria uma palavra de 16 bits, a partir de dois parametros de 1 bit
 */

// Função base
int main()
{
    /* Definindo variáveis */
    char nome_arquivo[50], porta_cliente_origem[6];
    FILE *arquivo;
    WSADATA data;
    SOCKET sock_cliente;
    struct sockaddr_in sock_servidor, sock_cliente_destino;
    int addr_tam = sizeof(SOCKADDR), size_receiving_message, status, packet_counter, porta_cliente;
    char *buffer, checksum_helper[3];
    buffer = (char *)malloc(tam_buffer);
    /* Fim */

    if (winSockInit(&data, &sock_cliente))
    {
        //Dados do socket do servidor
        memset(&sock_servidor, 0, sizeof(sock_servidor)); // Zerando valores para 'sock_servidor' (evitar lixo de memória)
        sock_servidor.sin_family = AF_INET; // Definindo como padrão para a comunicação o IPV4
        sock_servidor.sin_port = htons(porta_servidor); // Setando a porta
        sock_servidor.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP onde irá rodar o servidor

        /* Inicializando LOOP que será responsável pela busca/recebimento de dados do arquivo */
        while (1)
        {
            /** Solicitando dados do arquivo **/
            printf("Insira o nome do arquivo\n");
            scanf("%s", nome_arquivo);
            strcpy(buffer, nome_arquivo); // Seta o nome do arquivo a um buffer, que serve para ser enviado ao servidor
            /** Fim **/

            /** Envia o nome do arquivo ao servidor e captura um possível erro **/
            if (sendto(sock_cliente, buffer, strlen(buffer), 0, (struct sockaddr *)&sock_servidor, sizeof(sock_servidor)) == SOCKET_ERROR)
            {
                printf("sendto() falhou. Codigo : %d\n", WSAGetLastError());
                exit(EXIT_FAILURE); // Para a execução do programa, caso ocorra algum erro
            }
            /** Fim **/
            printf("Aguardando resposta do servidor...\n");
            status = 0;
            /** Loop para receber/processar resposta do servudor (caminho para o client que possuí o arquivo desejado) **/
            while (1)
            {
                memset(buffer, '\0', tam_buffer); // Seta o buffer, que antes contia nosso nome de arquivo, como vazio

                /*** Tenta receber os dados do servidor rastreador ***/
                if ((size_receiving_message = recvfrom(sock_cliente, buffer, tam_buffer, 0, (struct sockaddr *)&sock_servidor, &addr_tam)) == SOCKET_ERROR)
                {
                    printf("recvfrom() falhou. Codigo : %d\n", WSAGetLastError());
                    exit(EXIT_FAILURE); // Para a execução do programa, caso ocorra algum erro
                }
                /*** Fim ***/
                if (buffer[0] != '\0') // Se o buffer for diferente de vazio
                {
                    /***
                     * Nesse ponto, buffer[0] diz respeito ao estado da requisição:
                     * 1 -> Sucesso
                     * 0 -> Erro
                     *******************************************************************
                     * E, buffer[1] é a resposta em si, que pode ser:
                     * Se buffer[0] == 1 -> Dados do client que envia, necessário para obtermos o arquivo desejado
                     * Se buffer[0] == 0 -> Mensagem de erro que o servidor retornou
                     ***/
                    if (buffer[0] == '1')
                    {
                        buffer[1] = '1';
                        strcpy(porta_cliente_origem, &buffer[1]);
                        status = 1; // Seta flag para dizer que informações sobre o arquivo desejado, foi recebido
                        printf("Dados do rastreador recebidos\n");
                        printf("Porta cliente origem: %s\nAguarde...\n", porta_cliente_origem);
                        sleep(3);
                        break;
                    }
                    else if (buffer[0] == '0')
                    {
                        printf("Ops, o arquivo '%s' não foi localizado. Erro: \n%s\n", nome_arquivo, &buffer[1]);
                        break;
                    }
                }
            }
            /*** Se a requisição não retornar erro ***/
            if (status == 1)
            {
                printf("Solicitando arquivo no cliente fonte"); // Imprime uma mensagem que está solicitando o arquivo no cliente envia
                break; // Para o laço de repetição while, que aguarda resposta de uma solicitação ao servidor
            }
        }
        /* Fim */
        /* Solicitando arquivo no cliente envia */
        //Dados do socket do cliente envia
        memset(&sock_cliente_destino, 0, sizeof(sock_cliente_destino)); // Zerando valores para 'sock_cliente_destino' (evitar lixo de memória)
        sock_cliente_destino.sin_family = AF_INET; // Definindo como padrão para a comunicação o IPV4
        porta_cliente = atoi(porta_cliente_origem); // Transforma a string 'porta_cliente_origem', em um inteiro válido e salva em 'porta_cliente'
        sock_cliente_destino.sin_port = htons(porta_cliente); // Setando a porta
        sock_cliente_destino.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP do cliente que envia

        memset(buffer, '\0', tam_buffer); // Seta vazio ao buffer
        strcpy(buffer, nome_arquivo); // Adiciona o nome do arquivo final, ao buffer, que seja enviado na solicitação
        /** Envia o nome do arquivo ao servidor e captura um possível erro **/
        if (sendto(sock_cliente, buffer, strlen(buffer), 0, (struct sockaddr *)&sock_cliente_destino, sizeof(sock_cliente_destino)) == SOCKET_ERROR)
        {
            printf("'sendto()' falhou. Codigo : %d\n", WSAGetLastError());
            exit(EXIT_FAILURE); // Para a execução do programa, caso ocorra algum erro
        }
        /*** Laço de recebimento de dados do client destino ***/
        while (1)
        {
            memset(buffer, '\0', tam_buffer); // Seta vazio ao buffer
            if ((size_receiving_message = recvfrom(sock_cliente, buffer, tam_buffer, 0, (struct sockaddr *)&sock_cliente_destino, &addr_tam)) == SOCKET_ERROR)
            {
                printf("'recvfrom()' falhou. Codigo : %d\n", WSAGetLastError());
                exit(EXIT_FAILURE); // Para a execução do programa, caso ocorra algum erro
            }
            if (buffer[0] != '\0') // Se o buffer for diferente de vazio
            {
                if (strcmp(buffer, "5") == 0) // Se não houverem erros na requisição
                {
                    printf("Arquivo localizado\nIniciando transferencia do arquivo %s\n", nome_arquivo);
                    arquivo = fopen(nome_arquivo, "wb"); // Cria um arquivo, do tipo binário, para escrita ('wb')
                    break; // Para o laço de recebimento de dados do client destino
                }
                else if (strcmp(buffer, "0connect: no such file or directory") == 0) // Se o arquivo não foi encontrado no cliente destino
                {
                    printf("Aquivo '%s' não localizado\n%s\n", nome_arquivo, &buffer[1]); // Imprime mensagem de erro enviada pelo cliente
                }
            }
        }
        /*** Fim ***/
        packet_counter = 0; // Inicializa a contagem de pacotes recebidos
        /*** Inicializa recebimento do arquivo ***/
        while (1)
        {
            memset(buffer, '\0', tam_buffer); // Seta vazio ao buffer
            if ((size_receiving_message = recvfrom(sock_cliente, buffer, tam_buffer, 0, (struct sockaddr *)&sock_cliente_destino, &addr_tam)) == SOCKET_ERROR)
            {
                printf("recvfrom() falhou. Codigo : %d\n", WSAGetLastError());
                exit(EXIT_FAILURE); // Para a execução do programa, caso ocorra algum erro
            }
            if (buffer[0] != '\0') // Se o buffer for diferente de vazio
            {
                if (buffer[0] == '1') // Há pacotes para receber
                {
                    checksum(buffer, size_receiving_message, checksum_helper); // Realiza o checksum do pacote e salva na variável 'ch'
                    if (checksum_helper[0] == buffer[2] && checksum_helper[1] == buffer[3]) // Verifica se os checksums enviados, batem com o aqui calculado, após o recebimento
                    {
                        fwrite(&buffer[4], 1, size_receiving_message - 4, arquivo); // Escreve o conteúdo recebido do pacote para o arquivo previamente criado
                        packet_counter++; // Adiciona mais uma contagem de pacotes recebidos
                        strcpy(buffer, "ack1"); // Adiciono ao buffer de envio a mensagem de 'ack1', em outras palavras, sucesso ao receber o pacote X
                        printf("mensagem = %s\n", buffer);
                        /**** Envio o conteúdo do ack de volta ao server ****/
                        if (sendto(sock_cliente, buffer, strlen(buffer), 0, (struct sockaddr *)&sock_cliente_destino, sizeof(sock_cliente_destino)) == SOCKET_ERROR)
                        {
                            printf("'sendto()' falhou. Codigo : %d\n", WSAGetLastError());
                            exit(EXIT_FAILURE); // Para a execução do programa, caso ocorra algum erro
                        }
                        /**** Fim ****/
                        printf("Pacote %d recebido\n", packet_counter); // Imprime uma mensagem de sucesso que nos informa que recebemos o conteúdo e nos mostra o número do respectivo pacote
                    }
                    else // Caso o pacote não esteja integro
                    {
                        strcpy(buffer, "ack0"); // Adiciono ao buffer de envio a mensagem de 'ack0', em outras palavras, erro ao receber o pacote X
                        /**** Envio o conteúdo do ack de volta ao server ****/
                        if (sendto(sock_cliente, buffer, strlen(buffer), 0, (struct sockaddr *)&sock_cliente_destino, sizeof(sock_cliente_destino)) == SOCKET_ERROR)
                        {
                            printf("'sendto()' falhou. Codigo : %d\n", WSAGetLastError());
                            exit(EXIT_FAILURE); // Para a execução do programa, caso ocorra algum erro
                        }
                        /**** Fim ****/
                    }
                }
                else if (buffer[0] == '0') // Último pacote a ser recebido
                {
                    checksum(buffer, size_receiving_message, checksum_helper); // Realiza o checksum do pacote e salva na variável 'ch'
                    if (checksum_helper[0] == buffer[2] && checksum_helper[1] == buffer[3])// Verifica se os checksums enviados, batem com o aqui calculado, após o recebimento
                    {
                        fwrite(&buffer[4], 1, size_receiving_message - 4, arquivo); // Escreve o conteúdo recebido do pacote para o arquivo previamente criado
                        packet_counter++; // Adiciona mais uma contagem de pacotes recebidos
                        strcpy(buffer, "ack1"); // Adiciono ao buffer de envio a mensagem de 'ack1', em outras palavras, sucesso ao receber o pacote X
                        /**** Envio o conteúdo do ack de volta ao server ****/
                        if (sendto(sock_cliente, buffer, strlen(buffer), 0, (struct sockaddr *)&sock_cliente_destino, sizeof(sock_cliente_destino)) == SOCKET_ERROR)
                        {
                            printf("'sendto()' falhou. Codigo : %d\n", WSAGetLastError());
                            exit(EXIT_FAILURE); // Para a execução do programa, caso ocorra algum erro
                        }
                        /**** Fim ****/
                        /**** Imprime informações referentes ao arquivo transferido ****/
                        printf("Pacote %d-FINAL recebido\n", packet_counter);
                        printf("Transferencia do arquivo '%s' concluida\n", nome_arquivo);
                        /**** Fim ****/
                        break; // Para o laço de recebimento. Não há mais pacotes a serem lidos
                    }
                    else // Caso o pacote não esteja integro
                    {
                        strcpy(buffer, "ack0"); // Adiciono ao buffer de envio a mensagem de 'ack0', em outras palavras, erro ao receber o pacote X
                        /**** Envio o conteúdo do ack de volta ao server ****/
                        if (sendto(sock_cliente, buffer, strlen(buffer), 0, (struct sockaddr *)&sock_cliente_destino, sizeof(sock_cliente_destino)) == SOCKET_ERROR)
                        {
                            printf("'sendto()' falhou. Codigo : %d\n", WSAGetLastError());
                            exit(EXIT_FAILURE); // Para a execução do programa, caso ocorra algum erro
                        }
                        /**** Fim ****/
                    }
                }
            }
        }
        /*** Fim ***/
        fclose(arquivo); // Fecha o arquivo criado para recebimento
        /* Fim */
    }
    else
    {
        printf("Erro ao realizar o processo de inicio do socket");
    }
    system("pause"); // Finaliza o programa, pois o mesmo já chegou ao fim
}

/* Implementando funções com cabeçalho pré-definido */
int checksum(char *buffer_read, int tam_msg, char *check)
{
    unsigned int seed = 0, i;
    char checksum_helper[3];
    for (i = 4; i < tam_msg; i++)
    {
        seed += buffer_read[i];
    }

    /* Convertendo para base decimal */
    seed = seed % 16;
    if (seed < 10)
    {
        seed = seed * 10;
    }
    printf("Checksum = %d\n", seed);
    itoa(seed, checksum_helper, 10);
    /* Fim */

    //Setando valores de checksum
    check[0] = checksum_helper[0];
    check[1] = checksum_helper[1];
    return 0;
}

int winSockInit(WSADATA *data, SOCKET *sock_cliente)
{
    //Iniciando sokets em windows
    int startSockWinStats = startSockWin(data);
    //criando socket local
    int createLocalSockStats = createLocalSock(sock_cliente);
    return (startSockWinStats && createLocalSockStats);
}

int startSockWin(WSADATA *data)
{
    if (WSAStartup(MAKEWORD(2, 2), data) != 0)
    {
        printf("Falha ao fazer WSAStartup\n");
        return 0;
    }

    return 1;
}

int createLocalSock(SOCKET *sock_cliente)
{
    *sock_cliente = socket(AF_INET, SOCK_DGRAM, 0);
    if (*sock_cliente < 0)
    {
        printf("Erro ao iniciar socket\n");
        return 0;
    }
}
/* Fim */