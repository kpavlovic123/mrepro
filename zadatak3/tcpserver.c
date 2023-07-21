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

#define MYPORT 1234
#define BACKLOG 10
#define MAX_SIZE 1024

int main(int argc, char* argv[]) {
	int ch;
	uint16_t port = MYPORT;
	while((ch=getopt(argc,argv,"p:"))!=-1){
		switch(ch){
			case 'p':
				port = atoi(optarg);
				break;
			default:
				printf("Usage: ./tcpserver [-p port]");
				exit(1);
				break;
		}
	}
	int serverfd, clientfd;
	struct sockaddr_in serverSock, clientSock;
	
	memset(&serverSock, 0, sizeof(serverSock));
	
	serverSock.sin_port = htons(port);
	serverSock.sin_family = AF_INET;
	serverSock.sin_addr.s_addr = INADDR_ANY;

	if((serverfd = socket(AF_INET, SOCK_STREAM, 0))==-1){
		err(1,"socket");
	}

	if(bind(serverfd, (struct sockaddr*) &serverSock, sizeof(serverSock))==-1){
		err(1,"bind");		
	}	
	
	if(listen(serverfd,BACKLOG)==-1){
		err(1,"listen");	
	}
	
	printf("Listening...\n");

	int client_size = sizeof clientSock;
	while(1){
		if((clientfd = accept(serverfd,(struct sockaddr *)&clientSock,&client_size))==-1){
			err(1,"accept");
		}

		char hostName[256],serviceName[10];
		
		if (getnameinfo((struct sockaddr *)&clientSock, client_size,
                              hostName, sizeof(hostName), serviceName, sizeof(serviceName),
                              NI_NOFQDN | NI_NUMERICSERV)!=0) {
			err(1,"getnameinfo");
    		}

		printf("New connection from %s:%s\n", hostName,serviceName);
		

		char buf[MAX_SIZE] = {0};
		int n; 
		if((n=read(clientfd,buf,sizeof buf))==-1){
			err(1,"read");
		}

		int offset;
		memcpy(&offset,buf,4);
		offset = ntohl(offset);
		char* filename = &buf[4];
		
		printf("Received message from client. Size: %d Offset: %d Message: %s\n",
				n,offset,filename);
		
		int file_exists = access(filename,F_OK);
		int file_reads = access(filename,R_OK);

		if(file_exists != 0 || strchr(&buf[4],'/')!=NULL){
			char* msg = "File doesn't exist.";
			printf("%s\n",msg);
			char res[sizeof (msg) +1];
			res[0] = 1;
			strcpy(&res[1],msg);
			if(write(clientfd,res,sizeof(res))==-1){
				err(1,"write");
			}
			close(clientfd);	
		}
		else if(file_reads != 0){
			char* msg = "Restricted access to reading.";
			printf("%s\n",msg);
			char res[sizeof (msg)+1];
			res[0]=2;
			strcpy(&res[1],msg);
			if(write(clientfd,res,sizeof(res))==-1){
				err(1,"write");
			}
			close(clientfd);
		}
		else{
			FILE *file;
			if((file = fopen(filename,"r"))==NULL){
				char* msg = "File opening failed.";
				printf("%s\n",msg);
				char res[sizeof (msg)+1];
				res[0]=3;
				strcpy(&res[1],msg);
				if(write(clientfd,res,sizeof(res))==-1){
					err(1,"write");
				}
				err(1,"open");
			}

			if(fseek(file,0,SEEK_END)!=0){
				err(1,"seek");
			}
			uint64_t size = ftell(file)-offset;

			if(fseek(file,offset,SEEK_SET)!=0){
				err(1,"seek");
			}
			
			char res[MAX_SIZE];
			res[0]=0;
			if(write(clientfd,res,1)==-1){
				err(1,"write");
			}
			while((n = fread(res,sizeof (char),sizeof res,file))>0){
				if(write(clientfd,res,n)==-1){
					err(1,"write");
				}
			}
			if(n==-1){
				err(1,"read");
			}
			printf("File content sent, closing connection...\n");
			fclose(file);
			close(clientfd);
		}
	}
}
