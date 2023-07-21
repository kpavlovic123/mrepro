#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <getopt.h>
#define MAXLEN 512

char* port = "1234";
char payload[MAXLEN] = "";

int main(int argc, char *argv[])
{
    int ch;
    while((ch = getopt(argc,argv,"l:p:")) != -1)
    {
        switch (ch){
            case 'l':
                port = optarg;
                printf("Port changed to: %s\n",port);
                break;
            case 'p':
                strcpy(payload,optarg);
                printf("Payload changed to: %s\n",payload);
                break;
            default:
                printf("Usage: ./UDP_server [-l port] [-p payload]\n");
                exit(1);
                break;
        }
    }

    int serverfd;
    struct addrinfo hints,*res;
         
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL,port,&hints,&res);

    serverfd = socket(res->ai_family,res->ai_socktype,res->ai_protocol);

    bind(serverfd,res->ai_addr,res->ai_addrlen);

    struct sockaddr cstruct;
    int cstructlen = sizeof cstruct;
    char buf[256] = {'\0'};

    while(1){
        int mlen = recvfrom(serverfd,buf,sizeof buf, 0 ,&cstruct,&cstructlen);
        printf("Message received: %s\n",buf);
        if(strcmp("HELLO",buf)==0){
            strcat(payload,"\n");
            sendto(serverfd,payload,strlen(payload),0,&cstruct,cstructlen);
            printf("Sent message: %s\n",payload);
        }
        memset(buf,0,sizeof buf);
    }

    return 0;
}


