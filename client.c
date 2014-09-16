
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<unistd.h>	//close
#include<fcntl.h>
#include<sys/stat.h>
#include<time.h>
#include"cliser.h"

//#define MAX_PACKET_SIZE	5120
#define MAX_PACKET_SIZE	1024

/* argv[0]	
 * argv[1]	Remote filename with absolute path (File to be retrived from server)
 * argv[2]	Output FileName with absolute path (File to be stored locally)
 * argv[3]	Remote Sever Ip Address
 */

int main(int argc,char *argv[])
{
	int sd,len,ofd;
	struct sockaddr_in servAddr;
	char *ch;
	char ch1[512];
	char ch2[512];
	char tch2[512];
	unsigned long int fileSize,TotalBytesReceived;
	struct timespec t1;
	struct uname u1;

	t1.tv_sec=0;
	t1.tv_nsec=1;
	
	if (argc != 4 ) {
		printf("Usage: ./client inputfilename(serverfilename) outputfilename(localfilename with absolute path) serverIPADDRESS\n");
		exit(-1);
	}
	//ofd=open("/tmp/testbin",O_CREAT|O_WRONLY);
	//ofd=open(argv[2],O_CREAT|O_WRONLY,S_IRWXU);
	ofd=open(argv[2],O_CREAT|O_WRONLY,0644);
	/* Socket Creation */ 
	if ( (sd = socket(AF_INET,SOCK_STREAM,0)) < 0 ) {
		perror("socket: ");
		exit(-1);
	}
	
	servAddr.sin_family = AF_INET;
	//servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_addr.s_addr = inet_addr(argv[3]);
	servAddr.sin_port=htons(5000);

	if ( connect(sd,(struct sockaddr *)&servAddr,sizeof(servAddr)) < 0 ) {
		perror("connect ");
		exit(0);
	}
	#if 1
	/* Response from server for USER AUTHENTICATION */
	printf("Enter Username: ");
	scanf("%s",u1.username);
	printf("Enter Password: ");
	//scanf("%s",u1.password);
	strcpy(u1.password,getpass(""));
	len=write(sd,&u1,sizeof(u1));

	memset(ch2,'\0',512);
	len=read(sd,ch2,512);
	printf("Server Response for user authentication: %s and len(ch2):%d\n",ch2,len);
	ch2[strlen(ch2)]='\0';

	if (strcmp(ch2,"UserAuthentication Failed...") == 0 ) {
		printf("Authentication Failed...\n");
		unlink(argv[2]);
		exit(0);
	}

	#endif
	ch=argv[1];
	len=strlen(ch);
	//printf("Sent Message :%s\n",buf);
	//sendto(sd,&buf,1,0,(struct sockaddr *)&servAddr,sizeof(servAddr));
	/* Sending File name to be retrived from server */
	len=write(sd,ch,len);
	//printf("Reading back (File Content) from same socket...\n");
	printf("Reading back (File Size) from same socket...\n");
	/* ch1 is either FILENOT AVAILABLE or size of the requested file */
	len=read(sd,ch1,512);
	//len=recv(sd,&tmp,2,0);
	printf("read succesful with :%d bytes(fileSize info) \n",len);
	//printf("string length of ch1:%d\n",strlen(ch1));
	ch1[strlen(ch1)]='\0';
	printf("File Size from server :%s\n",ch1);

	strcpy(tch2,ch1);
	if (strstr(tch2,"Err|") != NULL ) {
		printf("Authentication Failed...with :%s\n",ch1);
		unlink(argv[2]);
		exit(0);
	}

	fileSize=atoll(ch1);
	printf("(after atoi func)File Size fileSize:%lu\n",fileSize);

	TotalBytesReceived=0;
	ch = (char *) malloc (sizeof(char) * MAX_PACKET_SIZE);
	if (fileSize <= MAX_PACKET_SIZE) {
		len=read(sd,ch,fileSize);
		printf("SMALL FILE....................................\n");
		printf("read succesful with :%d bytes\n",len);
		printf("Writing %lu bytes of filecontent to localfile\n",fileSize);
		//printf("%s",ch);
		len=write(ofd,ch,fileSize);
		printf("Written %d bytes of filecontent to localfile\n",len);
		memset(ch,'\0',fileSize);
	} else {
		while (TotalBytesReceived < fileSize) {
			len=read(sd,ch,MAX_PACKET_SIZE);
			//printf("read length:%d\n",len);
			/*
			if (len != MAX_PACKET_SIZE ) {
				printf("WRONG...(actual received):%d\n",len);
				//getchar();
			}
			*/
			len=write(ofd,ch,len);
			//nanosleep(&t1,NULL);
			//usleep(1);
			//TotalBytesReceived+=MAX_PACKET_SIZE;
			TotalBytesReceived+=len;
			//getchar();
			memset(ch,'\0',MAX_PACKET_SIZE);
			//printf("Total bytes received:%ld fileSize:%ld\n",TotalBytesReceived,fileSize);
		}
	}
	printf("transfer done\n");
	close(ofd);
	//getchar();
	free(ch);
	printf("before closing the socket...\n");
	close(sd);
	return 0;
}


