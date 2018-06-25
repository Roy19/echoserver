#ifndef _net_h_
#define _net_h_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <lcthw/ringbuffer.h>
#include <lcthw/bstrlib.h>
#include <lcthw/dbg.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BACKLOG	10

int get_socket(const char * host, const char * port);
int readSome(RingBuffer * buff, int fd, int is_socket);
int writeSome(RingBuffer * buff, int fd, int is_socket);
void child_handler(int s);
void * get_in_addr(struct sockaddr * sa);
int set_non_block(int fd);
void serve_connection(int child_fd);
int echo_server(const char * host, const char * port);

#endif
