/* 
 * atualiza_ip.c --- atualiza o ip do gateway no banco de dados
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
 * Documentação disponível no repositório.
 * 
 * Code:
 */

#include "atualiza_ip.h"

// Abre um socket stream tcp no host e na porta port. Retorna 0 para
// erro e 1 para sucesso.
int net_connect(char *host, int port)
{
        struct hostent *he;
        struct sockaddr_in their_addr;
        
        if ((he = gethostbyname(host)) == NULL) {
                herror("gethostbyname");
                return 0;
        }

        if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
                perror("socket");
                return 0;
        }

        their_addr.sin_family = AF_INET;
        their_addr.sin_port = htons(PORTA);
        their_addr.sin_addr = *((struct in_addr *)he->h_addr);
        bzero(&(their_addr.sin_zero), 8);

        if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
                perror("connect");
                return 0;
        }
        
        return 1;
}

// Envia uma string para o socket aberto.
int net_send(const char* msg)
{
        if (send(sockfd, msg, strlen(msg), 0) == -1) {
                perror("send");
                return 0;
        }

        return 1;
}

// Recebe uma string pelo socket aberto.
char *net_rec()
{
        char *buf;
        int numbytes;
	fd_set fds;
	struct timeval tv;

        buf = malloc(128);
        memset(buf, 0, 128);

	tv.tv_sec = 30;
	tv.tv_usec = 0;
	FD_ZERO (&fds);
	FD_SET (sockfd, &fds);
	if (select (sockfd+1, &fds, NULL, NULL, &tv)) {
                if ((numbytes = recv(sockfd, buf, 128, 0)) == -1) {
                        perror("recv");
                        return NULL;
                }
                
                buf[numbytes] = '\0';
        
                return buf;
        } else {
                if (verbose) {
                        fprintf(stdout, "%snet_rec Tempo limite esgotado.\n", data());
                        fflush(stdout);
                }
                return NULL;
        }
}

// Fecha o socket aberto.
void net_close()
{
        close(sockfd);
}

// Pega o IP de saída de onde este programa estiver rodando pelo
// webserver definido em HOSTNAME.
char* ip_no_webserver()
{
        char *ip;
        const char *request = "GET /meuip/index.php\r\n\r\n";

        if (!net_connect(HOSTNAME, PORTA)) {
                if (verbose) {
                        fprintf(stdout, "%sNão foi possivel estabelecer uma conexão com o servidor remoto.\n", data());
                        fflush(stdout);
                }
                return NULL;
        }
        
        if (!net_send(request)) {
                if (verbose) {
                        fprintf(stdout, "%sNão foi possivel enviar msg.\n", data());
                        fflush(stdout);
                }
                return NULL;
        }

        ip = net_rec();
        if (!ip) {
                if (verbose) {
                        fprintf(stdout, "%sNão recebemos dados.\n", data());
                        fflush(stdout);
                }
                return NULL;
        }

        net_close();

        return ip;
}

// Verifica se o arquivo existe.
int arquivo_existe(char *arquivo)
{
        struct stat buf;

        int i = stat(arquivo, &buf);
        if (i) {
                return 0;
        }

        return 1;
}

// Retorna o home concatenado com o arquivo de registro.
char* arquivo_completo()
{
        char *arquivo_completo;
        char *home_dir;        

        home_dir = getenv("HOME");
        if (!home_dir) {
                struct passwd *pw = getpwuid(getuid());
                home_dir = pw->pw_dir;
        }

        arquivo_completo = malloc(strlen(home_dir) + strlen(ARQUIVO) + 1);
        memset(arquivo_completo, 0, sizeof(arquivo_completo));

        strcpy(arquivo_completo, home_dir);
        strcat(arquivo_completo, ARQUIVO);
        
        return arquivo_completo;
}

// Cria o arquivo de registro caso ele não exista.
int cria_arquivo_se_nao_existir()
{
        FILE *fd;
        char *arquivo = arquivo_completo();
        const char* ip_diferente_do_atual = "255.255.255.255";

        if (!arquivo_existe(arquivo)) {
                if (verbose) {
                        fprintf(stdout, "%sArquivo de registro não existe, criando %s.\n", data(), arquivo);
                        fflush(stdout);
                }

                fd = fopen(arquivo, "w");
                if (!fd) {
                        perror("fopen");
                        if (verbose) {
                                fprintf(stdout, "%sNão foi possivel criar o arquivo %s.\n", data(), arquivo);
                                fflush(stdout);
                        }
                        free(arquivo);
                        return 0;
                }
                
                if (!fwrite(ip_diferente_do_atual, sizeof(char), strlen(ip_diferente_do_atual), fd)) {
                        if (verbose) {
                                fprintf(stdout, "%sNão foi possivel escrever no arquivo %s.\n", data(), arquivo);
                                fflush(stdout);
                        }
                        free(arquivo);
                        return 0;
                } else {
                        if (verbose) {
                                fprintf(stdout, "%sIP diferente do atual (%s) registrado no arquivo.\n", data(), ip_diferente_do_atual);
                                fflush(stdout);
                        }
                }
                
                fclose(fd);
        }
        
        free(arquivo);
        
        return 1;
}

// Retorna o conteudo (IP) do arquivo de registro.
char* ip_do_arquivo()
{
        FILE *fd;
        char *arquivo = arquivo_completo();
        char* ip;

        ip = malloc(128);        
        if (!ip) {
                if (verbose) {
                        fprintf(stdout, "%sNão foi possivel alocar memoria.\n", data());
                        fflush(stdout);
                }
        }
        memset(ip, 0, 128);
        
        fd = fopen(arquivo, "r");
        if (!fd) {
                perror("fopen");
                if (verbose) {
                        fprintf(stdout, "%sNão foi possivel abrir o arquivo %s.\n", data(), arquivo);
                        fflush(stdout);
                }
                free(arquivo);
                return NULL;
        }
        
        if (!fread(ip, sizeof(char), 128, fd)) {
                if (verbose) {
                        fprintf(stdout, "%sNão foi possivel ler o arquivo %s.\n", data(), arquivo);
                        fflush(stdout);
                }
                free(arquivo);
                return NULL;
        }
        
        free(arquivo);
        fclose(fd);

        return ip;
}

// Escreve um IP no arquivo de registro.
int atualiza_arquivo(char *ip)
{
        FILE *fd;
        char *arquivo = arquivo_completo();
        
        fd = fopen(arquivo, "w");
        if (!fd) {
                perror("fopen");
                if (verbose) {
                        fprintf(stdout, "%sNão foi possivel abrir o arquivo %s.\n", data(), arquivo);
                        fflush(stdout);
                }
                free(arquivo);
                return 0;
        }
                
        if (!fwrite(ip, sizeof(char), strlen(ip), fd)) {
                if (verbose) {
                        fprintf(stdout, "%sNão foi possivel escrever no arquivo %s.\n", data(), arquivo);
                        fflush(stdout);
                }
                free(arquivo);
                return 0;
        }
        
        fclose(fd);        
        free(arquivo);

        return 1;
}

// Atualiza no banco de dados com o ip especificado.
int atualiza_banco(char *ip, char *host)
{
        const char *conninfo = "host='hmdb.vexcorp.com' dbname='vex' user='atualizaip' password='oAjXuK2Sip' connect_timeout=30";
        PGconn     *conn;
        PGresult   *res;
        char query[256];
        
        conn = PQconnectdb(conninfo);
        if (PQstatus(conn) != CONNECTION_OK) {
                if (verbose) {
                        fprintf(stdout, "%sConexão com o banco falhou: (%s).\n", data(), PQerrorMessage(conn));
                        fflush(stdout);
                }
                PQfinish(conn);
                return 0;
        }
        
        sprintf(query, "update hotspot set ip_dinamico_gw='%s' where hostname='%s'", ip, host);
        res = PQexec(conn, query);
        if (!res) {
                if (verbose) {
                        fprintf(stdout, "%sExecução da query falhou: (%s).\n", data(), PQerrorMessage(conn));
                        fflush(stdout);
                }
                PQclear(res);
                PQfinish(conn);
                return 0;
        } else {
                if (verbose) {
                        fprintf(stdout, "%sBanco atualizado: (%s).\n", data(), query);
                        fflush(stdout);
                }
        }
        
        PQclear(res);
        
        PQfinish(conn);

        return 1;
}

// Retorna uma data formata para logs
char* data()
{
        time_t t;
        
        time(&t);
        
        return ctime(&t);
}

// retorna o host da máquina
char* host_da_maquina()
{
        char* hostname = malloc(sizeof(char) * 128);
        if (!hostname) {
                if (verbose) {
                        fprintf(stdout, "%sNão foi possivel alocar memoria.\n", data());                
                        fflush(stdout);
                }
                return NULL;
        }
        memset(hostname, 0, sizeof(char) * 128);
        
        if (gethostname(hostname, sizeof(char) * 128)) {
                perror("gethostname");
                return NULL;
        }
        
        return hostname;
}

// imprime como usar o programa e sai
void usage(char *progname)
{
        fprintf(stdout, "Usage: %s [opções]\n", progname);
        fprintf(stdout, "\t -h \t Mostra a ajuda.\n");
        fprintf(stdout, "\t -v \t Ativa o log na saída padrão (stdout).\n");
}

int main(int argc, char **argv)
{
        char *ip_atual, *ultimo_ip, *hostname;
        int c;

        while ((c = getopt(argc, argv, "hv")) != -1) {
                switch (c) {
                case 'h':
                        usage(argv[0]);
                        return 0;
                case 'v':
                        verbose = 1;
                        break;
                case '?':
                        usage(argv[0]);
                        return 1;
                }
        }

        ip_atual = ip_no_webserver();
        if (!ip_atual) {
                if (verbose) {
                        fprintf(stdout, "%sNão foi possivel pegar o ip no servidor web.\n", data());
                        fflush(stdout);
                }
                return 1;
        } else {
                if (verbose) {
                        fprintf(stdout, "%sIP do webserver: (%s).\n", data(), ip_atual);
                        fflush(stdout);
                }
        }
        
        if (!cria_arquivo_se_nao_existir(ip_atual)) {
                if (verbose) {
                        fprintf(stdout, "%sNão foi possivel criar o arquivo de registro.\n", data());
                        fflush(stdout);
                }

                free(ip_atual);

                return 1;
        } else {
                if (verbose) {
                        fprintf(stdout, "%sArquivo de registro: OK.\n", data());
                        fflush(stdout);
                }
        }

        ultimo_ip = ip_do_arquivo();
        if (!ultimo_ip) {
                if (verbose) {
                        fprintf(stdout, "%sNão foi possivel pegar o IP do arquivo de registro.\n", data());
                        fflush(stdout);
                }

                free(ip_atual);

                return 1;
        } else {
                if (verbose) {
                        fprintf(stdout, "%sIp do arquivo de registro: (%s).\n", data(), ultimo_ip);
                        fflush(stdout);
                }
        }

        if (!strcmp(ultimo_ip, ip_atual)) {
                if (verbose) {
                        fprintf(stdout, "%sOs IPs são iguais: IP atual (%s) e IP registrado (%s).\n", data(), ip_atual, ultimo_ip);
                        fflush(stdout);
                }
        } else {
                if (verbose) {
                        fprintf(stdout, "%sOs IPs são diferentes: IP atual (%s) e IP registrado (%s).\n", data(), ip_atual, ultimo_ip);
                        fflush(stdout);
                }

                hostname = host_da_maquina();
                if (!hostname) {
                        if (verbose) {
                                fprintf(stdout, "%sNão foi possivel pegar o hostname da máquina.\n", data());
                                fflush(stdout);
                        }
                        
                        free(ip_atual);
                        free(ultimo_ip);

                        return 1;
                } else {
                        if (verbose) {
                                fprintf(stdout, "%sHostname da máquina: (%s).\n", data(), hostname);
                                fflush(stdout);
                        }
                }
                
                if (!atualiza_banco(ip_atual, hostname)) {
                        if (verbose) {
                                fprintf(stdout, "%s Não foi possível atualizar o banco.\n", data());
                                fflush(stdout);
                        }

                        free(ip_atual);
                        free(ultimo_ip);
                        free(hostname);

                        return 1;
                } else {
                        if (verbose) {
                                fprintf(stdout, "%sBanco atualizado para o IP (%s).\n", data(), ip_atual);
                                fflush(stdout);
                        }

                        if (!atualiza_arquivo(ip_atual)) {
                                if (verbose) {
                                        fprintf(stdout, "%sNão foi possivel atualizar o arquivo de registro.\n", data());
                                        fflush(stdout);
                                }

                                free(ip_atual);
                                free(ultimo_ip);
                                free(hostname);
                                
                                return 1;
                        } else {
                                if (verbose) {
                                        fprintf(stdout, "%sArquivo de registro atualizado para (%s).\n", data(), ip_atual);
                                        fflush(stdout);
                                }
                        }
                }

                free(hostname);
        }
        
        free(ip_atual);
        free(ultimo_ip);

        return 0;
}
