#undef NDEBUG
#include <net.h>

const int RB_SIZE = 1024 * 10;

void serve_connection(int child_fd){
	check(child_fd != -1, "Invalid child file descriptor.");
	RingBuffer * buff = RingBuffer_create(RB_SIZE);
	while(readSome(buff, child_fd, 1) != -1){
		if(writeSome(buff, child_fd, 1) == -1){
			debug("Connection closed.")
			close(child_fd);
			break;
		}
	}
error:	// fall through
	if(buff)	RingBuffer_destroy(buff);
	exit(0);
}

int main(int argc, char * argv[]){
        struct sockaddr_storage their_addr;
	socklen_t sin_size;
	struct sigaction sa;
	char ip[INET6_ADDRSTRLEN];
	int sockfd = -1, newfd = -1, rc;
	check(argc == 3, "USAGE: echo_server <host> <port>");
	
	sockfd = get_socket(argv[1], argv[2]);
	check(sockfd != -1, "Failed to get sockfd");

	check(listen(sockfd, BACKLOG) != -1, "Failed to listen on socket = %d",sockfd);
	sa.sa_handler = child_handler;
	sigemptyset(&sa.sa_mask);
	if(sigaction(SIGCHLD, &sa, NULL) == -1){
		perror("sigaction");
		goto error;
	}

	debug("Waiting for connections.........................");

	while(1){
		sin_size = sizeof(their_addr);
		newfd = accept(sockfd,(struct sockaddr *)&their_addr, &sin_size);
		if(newfd == -1){
			perror("accept");
			continue;
		}
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),
					ip, sizeof(ip));
		debug("Got a connection from %s", ip);
		rc = fork();
		if(rc == 0){
			// child process
			close(sockfd);
			serve_connection(newfd);			
		}else{
			// parent process
			close(newfd);
		}
	}
		
	return 0;
error:
	if(sockfd != -1)	close(sockfd);
	return -1;
}
