#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <getopt.h>

int ch;
char *host, *service, *adress, *port;
struct addrinfo hints, *res;
int order, hex;

void getAddr(char *argv[])
{
    int error;
    char addrstr[100];
    host = argv[optind];
    service = argv[optind + 1];
    error = getaddrinfo(host, service, &hints, &res);
    if (error)
    {
        errx(1, "%s", gai_strerror(error));
    }
    inet_ntop(res->ai_family, &((struct sockaddr_in *)res->ai_addr)->sin_addr, addrstr, 100);
    uint16_t port;
    if (!order)
        port = htons(((struct sockaddr_in *)res->ai_addr)->sin_port);
    else
        port = ntohs(((struct sockaddr_in *)res->ai_addr)->sin_port);

    if (!hex)
        printf("%s (%s) %d\n", addrstr, res->ai_canonname, port);
    else
        printf("%s (%s) %x\n", addrstr, res->ai_canonname, port);
    freeaddrinfo(res);
}

void getAddr6(char *argv[])
{
    int error;
    char addrstr[100];
    host = argv[optind];
    service = argv[optind + 1];
    error = getaddrinfo(host, service, &hints, &res);
    if (error)
    {
        errx(1, "%s", gai_strerror(error));
    }
    inet_ntop(res->ai_family, &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr, addrstr, 100);
    uint16_t port;
    if (!order)
        port = htons(((struct sockaddr_in6 *)res->ai_addr)->sin6_port);
    else
        port = ntohs(((struct sockaddr_in6 *)res->ai_addr)->sin6_port);

    if (!hex)
        printf("%s (%s) %d\n", addrstr, res->ai_canonname, port);
    else
        printf("%s (%s) %x\n", addrstr, res->ai_canonname, port);
    freeaddrinfo(res);
}

void reverse(char* argv[]){
    adress = argv[optind];
    port = argv[optind+1];
    char host[NI_MAXHOST],service[NI_MAXSERV];
    int error; 
    struct sockaddr_in sockaddr;
    sockaddr.sin_port = htons(atoi(port));
    sockaddr.sin_family = AF_INET;
    inet_pton(AF_INET,adress,&(sockaddr.sin_addr)); 
    int flag = NI_NAMEREQD;
    if(hints.ai_socktype = SOCK_DGRAM)
        flag = NI_DGRAM;
    error = getnameinfo((struct sockaddr *)&sockaddr,sizeof(struct sockaddr_in),host,sizeof(host),service,sizeof(service),flag);
    if (error) errx(1, "getnameinfo: %s", gai_strerror(error));
    printf("%s (%s) %s",adress,host,service);
}

void reverse6(char* argv[]){
    adress = argv[optind];
    port = argv[optind+1];
    char host[NI_MAXHOST],service[NI_MAXSERV];
    int error; 
    struct sockaddr_in6 sockaddr;
    sockaddr.sin6_port = htons(atoi(port));
    sockaddr.sin6_family = AF_INET6;
    inet_pton(AF_INET6,adress,&(sockaddr.sin6_addr)); 
    int flag = NI_NAMEREQD;
    if(hints.ai_socktype = SOCK_DGRAM)
        flag = NI_DGRAM;
    error = getnameinfo((struct sockaddr *)&sockaddr,sizeof(struct sockaddr_in6),host,sizeof(host),service,sizeof(service),flag);
    if (error) errx(1, "getnameinfo: %s", gai_strerror(error));
    printf("%s (%s) %s",adress,host,service);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        errx(1, "usage: %s hostname\n", argv[0]);
    }
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_CANONNAME;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int r = 0;
    while ((ch = getopt(argc, argv, "rtuxhn64")) != -1)
    {
        switch (ch)
        {

        case '4':
            break;

        case '6':
            hints.ai_family = AF_INET6;
            break;

        case 'r':
            r = 1;
            break;

        case 't':
            break;

        case 'u':
            hints.ai_socktype = SOCK_DGRAM;
            break;

        case 'x':
            hex = 1;
            break;

        case 'h':
            break;

        case 'n':
            order = 1;
            break;

        default:
	break;
        }
    }

    if (!r)
    {
        if (hints.ai_family == AF_INET)
            getAddr(argv);
        else
            getAddr6(argv);
    }
    else
        if(hints.ai_family == AF_INET)
            reverse(argv);
        else 
            reverse6(argv);
    return 0;
}
