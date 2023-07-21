#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define IP_ADDRESS "127.0.0.1"
#define PORT 1234
#define MAX_SIZE 1024


int main(int argc,char* argv[]){
	uint16_t port = PORT;
	char* ipAddress = IP_ADDRESS;
	int ch;
	int flagC = 0;
	while((ch=getopt(argc,argv,"s:p:c"))!=-1){
		switch(ch){
			case 's':
				ipAddress = optarg;
				break;
			case 'p':
				port = atoi(optarg);
				break;
			case 'c': 
				flagC = 1;
				break;
			default: 
				printf("Usage: ./tcpklijent [-s server] [-p port] [-c] filename");
				exit(1);
				break;
		}
	}

	if(optind == argc){
		printf("Usage: ./tcpklijent [-s server] [-p port] [-c] filename");
		exit(1);
	}
	char* filename = argv[optind];
	printf("Filename is: %s\n",filename);
	
	int offset = 0;
	int file_exists = access(filename,F_OK);

	if(flagC==0 && file_exists == 0){
		fprintf(stderr,"File already exists. Use -c flag.\n");
		exit(1);
	}
	else if(flagC==1 && access(filename,W_OK)!=0){
		fprintf(stderr,"Writing to file restricted.\n");
		exit(1);
	}
	else if(flagC==1){
		if(file_exists == 0){
			FILE* file;
			if((file = fopen(filename,"w"))==NULL){
				err(1,"open");		
			}
			if(fseek(file,0,SEEK_END)!=0){
				err(1,"seek");
			}
			offset = ftell(file);
			fclose(file);
		}
	}

	int clientfd;
	struct sockaddr_in serverSock;

	memset(&serverSock,0,sizeof serverSock);
	
	serverSock.sin_family = AF_INET;
	serverSock.sin_port = htons(port);
	if(inet_pton(AF_INET,ipAddress,&serverSock.sin_addr)==-1){
		err(1,"inet_pton");
	}

	if((clientfd = socket(AF_INET,SOCK_STREAM,0))<0){
		err(1,"socket");
	}

	if(connect(clientfd,(struct sockaddr*)&serverSock,sizeof serverSock)!=0){
		err(1,"connect");	
	}

	printf("Connected to server: %s:%d\n",ipAddress,port);
	
	char req[strlen(filename)+5];
	memcpy(req,&offset,4);
	memcpy(&req[4],filename,strlen(filename)+1);

	if(write(clientfd,req,sizeof req)<sizeof req){
		err(1,"write");
	}

	printf("Message sent to server. Offset: %d Message: %s\n",offset,&req[4]);

	char res[MAX_SIZE];

	int n;

	FILE* file;

	if((file = fopen(filename,"w"))==NULL){
		err(1,"open");
	}
	
	if(fseek(file,offset,SEEK_SET)!=0){
		err(1,"seek");
	}

	short status = -1;
	while((n=read(clientfd,res,sizeof res))>0){
		if(status == -1){
			status = res[0];
			if(status==0){
				int error = fwrite(&res[1],sizeof(char),n-1,file);
				if(error == -1){
					err(1,"write");
				}
			}
			else{
				printf("Message: %s\n",&res[1]);
				exit(status);
			}
		}
		else if(status == 0){
			if(fwrite(res,sizeof (char),n,file)<n){
				err(1,"write");
			}
		}	
			
	}
	if(n==-1){
		err(1,"read");
	}
	
	printf("Data stored to file.\nEnding client...\n");

	fclose(file);
	close(clientfd);
}
