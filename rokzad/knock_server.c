#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <getopt.h>
#include <sys/select.h>
#include <sys/time.h>

void startTCP(char *port, char *timeout);
void alarmHandler(int sig);
// Dodao volatile za signal interrupt
volatile int stop = 1;

int main(int argc, char *argv[])
{
    int ch;
    char *lozinka, u1, *t12, u2, *t23, u3, k4, k5;
    char *port = "1234";
    char *timeout = "10";

    while ((ch = getopt(argc, argv, "p:t:")) != -1)
    {
        switch (ch)
        {
        case 'p':
            port = optarg;
            break;
        case 't':
            timeout = optarg;
            break;
        default:
            u1, t12, u2, t23, u3, k4, k5;
            printf("Usage: knock_server [-p port] [-t timeout] lozinka u1 t12 u2 t23 u3 k4 k5\n");
            break;
        }
    }

    lozinka = argv[optind];
    t12 = argv[optind + 2];
    t23 = argv[optind + 4];

    char *udpPorts[5];
    udpPorts[0] = argv[optind + 1];
    udpPorts[1] = argv[optind + 3];
    udpPorts[2] = argv[optind + 5];
    udpPorts[3] = argv[optind + 6];
    udpPorts[4] = argv[optind + 7];

    int descriptors[5];

    struct sockaddr_in sockets[5];

    //U svakom for loopu maknuo sizeof 5
    for (int i = 0; i < 5; i++)
    {
        //Izmjenio socket<0 u descriptor[i]<0
        descriptors[i] = socket(AF_INET, SOCK_DGRAM, 0);
        if (descriptors[i] < 0)
        {
            err(1, "bind");
        }
    }

    for (int i = 0; i < 5; i++)
    {
        struct sockaddr_in s;
        memset(&s, 0, sizeof s);
        s.sin_addr.s_addr = INADDR_ANY;
        s.sin_port = htons(atoi(udpPorts[i]));
        //Dodavanje u sockets...
        sockets[i] = s;
    }

    for (int i = 0; i < 5; i++)
    {
        if (bind(descriptors[i], (struct sockaddr *)&sockets[i], sizeof sockets[i]) != 0)
        {
            err(1, "bind");
        }
    }

    fd_set readfds;

    int lock_index = 0;

    struct sockaddr_in client;
    socklen_t clientLen = sizeof client;
    memset(&client, 0, clientLen);

    while (1)
    {
        FD_ZERO(&readfds);
        // Promijenio i = 5 u i = 0
        for (int i = 0; i < 5; i++)
        {
            FD_SET(descriptors[i], &readfds);
        }

        struct timeval stop, start;
        gettimeofday(&start, NULL);
        int ready;
        // Dodao + 1 na max fd
        if ((ready = select(descriptors[4] + 1, &readfds, NULL, NULL, NULL)) == -1)
        {
            err(1, "select");
        }
        gettimeofday(&stop, NULL);
        if (FD_ISSET(descriptors[lock_index], &readfds))
        {
            // Maknuo "provjeru" isset za kontrolne udp portove

            /* if (FD_ISSET(descriptors[3], &readfds) == 1 || FD_ISSET(descriptors[4], &readfds) == 1)
            {
                lock_index = 0;
                continue;
            } */

            char buf[100];
            int n;
            if ((n = recvfrom(descriptors[lock_index], buf, sizeof buf, 0, (struct sockaddr *)&client, &clientLen)) == -1)
            {
                err(1, "recvfrom");
            }
            // Dodao printfove za pregledniju provjeru programa:lock reset, lock n passed i time diff
            if (lock_index == 0)
            {
                if (strcmp(buf, lozinka) != 0)
                {
                    printf("lock reset\n");
                    continue;
                }
                printf("lock 1 passed\n");
            }
            else if (lock_index == 1)
            {
                int diff = (stop.tv_sec - start.tv_sec) * 1000000 + (stop.tv_usec - start.tv_usec);
                int interval = atoi(t12) * 1000000;
                printf("time diff 12:%d\n", diff);
                if (!(interval + 500000 > diff && interval - 500000 < diff))
                {
                    lock_index = 0;
                    printf("lock reset\n");
                    continue;
                }
                printf("lock 2 passed\n");
            }
            else if (lock_index == 2)
            {
                int diff = (stop.tv_sec - start.tv_sec) * 1000000 + (stop.tv_usec - start.tv_usec);
                int interval = atoi(t23) * 1000000;
                printf("time diff 23:%d\n", diff);
                if (!(interval + 500000 > diff && interval - 500000 < diff))
                {
                    lock_index = 0;
                    printf("lock reset\n");
                    continue;
                }
                else
                {
                    printf("lock 3 passed\nOpened tcp server\n");
                    startTCP(port, timeout);
                    lock_index = 0;
                    continue;
                }
            }
            lock_index++;
        }
        else
        {
            // Dodao loop za unbufferanje deskriptora ako nije na trazenom indexu
            for (int i = 0; i < 5; i++)
            {
                if (FD_ISSET(descriptors[i], &readfds))
                {
                    char buf[100];
                    read(descriptors[i], buf, sizeof buf);
                }
            }
            printf("lock reset\n");
            lock_index = 0;
        }
    }
    return 0;
}

void startTCP(char *port, char *timeout)
{
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_PASSIVE;

    if (getaddrinfo(NULL, port, &hints, &res) == -1)
    {
        err(1, "getaddrinfo");
    }

    int serverFd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (serverFd < 0)
    {
        err(1, "socket");
    }

    // Dodao SO_REUSEADDR za izbjegavanje bind: Address already in use...
    int optval = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if (bind(serverFd, res->ai_addr, res->ai_addrlen) == -1)
    {
        err(1, "bind");
    }

    printf("Listening with tcp server...\n");
    if (listen(serverFd, 10) == -1)
    {
        err(1, "listen");
    }

    struct sigaction sigAct;

    sigAct.sa_handler = alarmHandler;

    if (sigaction(SIGALRM, &sigAct, NULL) == -1)
    {
        err(1, "sigaction");
    }

    alarm(atoi(timeout));

    while (stop)
    {
        struct sockaddr_in client;
        socklen_t clientLen = sizeof client;
        int clientFd;
        memset(&client, 0, clientLen);

        if ((clientFd = accept(serverFd, (struct sockaddr *)&client, &clientLen)) == -1)
        {
            // Dodao provjeru za alarm, inače program izađe
            if (stop == 0)
            {
                printf("Timeout expired.\n");
                break;
            }
            err(1, "accept");
        }
        char *msg = "Hello world";

        if (write(clientFd, msg, strlen(msg)) < 0)
        {
            err(1, "write");
        }

        close(clientFd);
    }
    // Dodao close i printf
    printf("Tcp server closed.\n");
    close(serverFd);
    stop = 1;
}

void alarmHandler(int sig)
{
    stop = 0;
}