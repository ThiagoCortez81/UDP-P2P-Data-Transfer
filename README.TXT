* ================================================================================= *
*                        FUNCIONAMENTO TESTADO NO WINDOWS 10                        *
* ================================================================================= *
Alunos:
 - Isaias Mendes Machado | 2017006498
 - Thiago Augusto da Silva Cortez | 2017005276

* ================================================================================= *
*                         INSTRUÇÕES PARA COMPILAR E RODAR                          *
* ================================================================================= *

 - PASTA "/"
 gcc -o serverUDP server.c -lws2_32 -lwsock32 -L $MingGW\lib
 gcc -o senderUDP sender.c -lws2_32 -lwsock32 -L $MingGW\lib

 - PASTA "/receiver"
 gcc -o receiverUDP receiver.c -lws2_32 -lwsock32 -L $MingGW\lib

                                            **** OBS ****
 Ao rodar o serverUDP:
  - Inserir '1', para poder adicionar novos arquivos ao rastreador (os arquivos permitidos são o que estão
  na mesma pasta do sender.c)
  - Quando solicitado a porta do cliente, especificar porta 10333 (porta que está rodando o cliente que 
  envia).

* ================================================================================= *
*                                     OBRIGADO                                      *
* ================================================================================= *