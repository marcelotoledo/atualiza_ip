/* 
 * atualiza_ip.h --- cabeçalho do atualiza_ip.c
 * 
 * Copyright  (C)  2005  VEX http://www.vexcorp.com
 * 
 * Version: 1.0
 * Keywords: 
 * Author: Marcelo Toledo <marcelo.toledo@vexcorp.com>
 * Maintainer: Marcelo Toledo <marcelo.toledo@vexcorp.com>
 * 
 * Commentary: 
 * 
 * 
 * Documentação disponível no repositório.
 * 
 * Code:
 */

#ifndef ATUALIZA_IP_H
#define ATUALIZA_IP_H

#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h> 
#include <sys/socket.h>
#include <pwd.h> 
#include <libpq-fe.h>
#include <stdarg.h>
#include <time.h>

#define PORTA 80
#define HOSTNAME "meuip.vexcorp.com"
#define ARQUIVO "/.atualiza_ip"

int net_connect(char *host, int port);
int net_send(const char* msg);
char *net_rec();
void net_close();
char* ip_no_webserver();
int arquivo_existe(char *arquivo);
char* arquivo_completo();
int cria_arquivo_se_nao_existir();
char* ip_do_arquivo();
int atualiza_arquivo(char *ip);
int atualiza_banco(char *ip, char *host);
char* data();
char* host_da_maquina();

int verbose = 0; // define se o programa está em modo verbose
int sockfd; // socket da conexão com webserver

#endif
