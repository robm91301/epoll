#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

// don't use magic numbers....define them here....
#define PORT 8081
#define MAXFDS 16 * 2048
#define N_BACKLOG 64
#define MAX_THREADS 8 
#define BUF_SIZE 1024

const int debug =0;
int efd;
int sd = 0;
typedef void *(*thread_fp) (void *);

// reverse the bits function found on the internet
uint8_t reverse(uint8_t b)
{
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

// another function found on the internet that
// tides up thread creation
int local_pthread_create(pthread_t * thread, thread_fp func, void *arg)
{
    int r;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    r = pthread_create(thread, &attr, func, arg);
    return r;
}

// a function that reads the data in
int read_data(int cfd, uint8_t * buf)
{

    int nbytes = recv(cfd, buf, BUF_SIZE, 0);
    if (nbytes == 0) {
	// The peer disconnected.
	perror("Closed Socket");
    }
    return nbytes;
}

// reverse and then write out the functions
void write_data(int cfd, uint8_t * buf, int nbytes)
{
    uint8_t *rev_buf = malloc(nbytes);
    for (int i = 0; i < nbytes; i++) {
	if (debug) printf("start: %x ", buf[i]);
	rev_buf[nbytes - i - 1] = reverse(buf[i]);
	if (debug) printf("reversed: %x \n", rev_buf[nbytes - i - 1]);
    }

    write(cfd, rev_buf, nbytes);
    free(rev_buf);
}

// thread function that waits around for input.
// started for each thread
void *thread_func(void *p)
{
    int r;
    int cfd;
    struct epoll_event e = { 0 };
    uint8_t buf[BUF_SIZE];

    // we are looping forever.....
    while (1) {
        // wait for a connection 
	r = epoll_pwait(efd, &e, 1, -1, 0);
	if (r == -1) {
	    // bail on all errors except EINTR
	    if (EINTR == errno) {
		continue;
	    } else {
		printf("epoll wait error \n");
		break;
	    }
	}
	// accept the connection and process it if we accept ok
	cfd = accept(sd, 0, 0);
	if (cfd == -1) {
	    return NULL;
	} else {
	    // read and write data....
	    int nbytes = read_data(cfd, buf);
	    write_data(cfd, buf, nbytes);
	}
	close(cfd);
    }
}

// function for making a socket into a listener...
// found on the internet
int listen_inet_socket(int portnum)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
	perror("ERROR opening socket");
    }
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) <
	0) {
	perror("setsockopt");
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portnum);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) <
	0) {
	perror("ERROR on binding");
    }

    if (listen(sockfd, N_BACKLOG) < 0) {
	perror("ERROR on listen");
    }

    return sockfd;
}

// main profram....void because we don't process the args....
int main(void)
{

    struct epoll_event event;
    int ret;
    const int true = 1;

    // create a epoll of size 1 since we only monitor one address 
    efd = epoll_create(1);
    struct epoll_event *events =
	calloc(MAXFDS, sizeof(struct epoll_event));
    if (efd < 0) {
	perror("epoll_create");
	exit(EXIT_FAILURE);
    }

    // create our listening socket...defaults to localhost
    sd = listen_inet_socket(PORT);
    if (sd <= 0) {
	perror("Failure to create listen sock\n");
	exit(EXIT_FAILURE);
    }
    // add our listening port for epoll
    event.data.fd = sd;		
    event.events = EPOLLIN;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, sd, &event);
    if (ret) {
	perror("epoll_ctl");
	exit(EXIT_FAILURE);
    }
    // create our threads and hang around for some input
    pthread_t thread;
    for (int i = 1; i < MAX_THREADS; i++)
	local_pthread_create(&thread, thread_func, NULL);

    thread_func(NULL);

}
