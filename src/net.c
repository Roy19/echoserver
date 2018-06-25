#include <net.h>

struct tagbstring NL = bsStatic("\n");
struct tagbstring CRLF = bsStatic("\r\n");

void child_handler(int s){
	(void)s;	// don't need s
	
	int saved_errno = errno;	// while terminating the child errno may change. 
					// so save it.
	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;		// restore errno
}

int set_non_block(int fd){
	int flags = fcntl(fd, F_GETFL, 0);	// Get the flags
	check(flags >= 0, "Invalid flags on nonblock.");

    	int rc = fcntl(fd, F_SETFL, flags | O_NONBLOCK);	// Use those flags and also set it
    	check(rc == 0, "Can't set nonblocking.");		// for Nonblocking I/O

    	return 0;
error:
    	return -1;
}

int get_socket(const char * host, const char * port){	
	struct addrinfo hints;
	struct addrinfo * res, * p;
	int sockfd, yes = 1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE;	// fill my IP for me
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	check(host || port, "Invalid parameters to host and port.");

	int rc = getaddrinfo(host, port, &hints, &res);
	check(rc == 0, "Could not lookup host:post : %s:%s", host, port);
	
	for(p = res; p != NULL; p = p->ai_next){
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))== -1){
			perror("Failed to initialize the socket.");
			continue;
		}
		if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
			perror("Failed to setsockopt().");
			close(sockfd);
			continue;
		}
		if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			perror("Failed to bind() to socket");
			close(sockfd);
			continue;
		}
		break;
	}
	check(p != NULL, "Could not get socket.");	// if we reached until here and can't create a socket
							// then it's probably an error
	freeaddrinfo(res);

	return sockfd;

error:
	if(res) freeaddrinfo(res);
	return -1;
}

void * get_in_addr(struct sockaddr * sa){
	check(sa != NULL, "Invalid struct sockaddr pointer.");

	if(sa->sa_family == AF_INET){
		return &((struct sockaddr_in *)sa)->sin_addr;
	}
	return &((struct sockaddr_in6 *)sa)->sin6_addr;
error:
	return NULL;
}

int readSome(RingBuffer * buffer, int fd, int is_socket){
	int rc = 0;

    	if (RingBuffer_available_data(buffer) == 0) {
	    	// BUG : use an interface here
        	buffer->start = buffer->end = 0;
    	}

    	if (is_socket) {	// If it's a socket use recv() to read into the RingBuffer 
        	rc = recv(fd, RingBuffer_starts_at(buffer),
                	RingBuffer_available_space(buffer), 0);
    	} else {		// else use read(). But here it's basically the same.
        	rc = read(fd, RingBuffer_starts_at(buffer),
                	RingBuffer_available_space(buffer));
    	}
	check(rc >= 0, "Failed to read from fd: %d", fd);
	RingBuffer_commit_write(buffer, rc);

    	return rc;

error:
    	return -1;
}

int writeSome(RingBuffer * buffer, int fd, int is_socket){
	int rc = 0;
    	bstring data = RingBuffer_get_all(buffer);

    	check(data != NULL, "Failed to get from the buffer.");
    	check(bfindreplace(data, &NL, &CRLF, 0) == BSTR_OK,		// Not much essential
        	    "Failed to replace NL.");

    	if (is_socket) {	// Same case as read_some
        	rc = send(fd, bdata(data), blength(data), 0);
    	} else {
        	rc = write(fd, bdata(data), blength(data));
    	}
	check(rc == blength(data), "Failed to write everything to fd: %d.",
        			    fd);
    	bdestroy(data);
	return rc;
error:
	return -1;
}
