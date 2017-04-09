SHELL = /bin/sh
CC = gcc
CFLAGS = -O3
INCLUDE = -I/usr/include/postgresql
LDFLAGS = -L/usr/local/pgsql/lib -lpq
#DEBUG = -g -DDEBUG
RM = /bin/rm -f

all: atualiza_ip

atualiza_ip:
	$(CC) $(CFLAGS) $(DEBUG) $(INCLUDE) -o atualiza_ip atualiza_ip.c $(LDFLAGS)

clean:
	$(RM) atualiza_ip
