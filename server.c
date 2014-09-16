
#include"server.h"
#include"cliser.h"

int main()
{
	int sd,nsd,ret,tosend;
	struct sockaddr_in servAddr,cliAddr;
	char ch[FILENAME_PATH];
	char ch1[10];
	unsigned long int fileSize,len,tmpFs,TotalBytesSent;
	char *buf;
	socklen_t clilen;
	struct stat fstat;
	FILE *fp;		//Client Requested file
	int opt=0;
	int EOFFlag=1;
	struct uname u1;
	int pid;
	
	
	/* Socket Creation */ 
	if ( (sd = socket(AF_INET,SOCK_STREAM,0)) < 0 ) {
		perror("socket: ");
		exit(-1);
	}
	/* Bind the socket wiht Known PORT # */	
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = INADDR_ANY;
	//servAddr.sin_addr.s_addr = inet_addr("172.17.0.120");
	servAddr.sin_port=htons(SERVER_PORT);
	if ( (bind(sd,(struct sockaddr *)&servAddr,sizeof(struct sockaddr_in ))) < 0 ) {
		perror("bind ");
		exit(0);
	}

	/* Crete a Queue for Incoming Connections */
	if ( (listen(sd,1)) < 0 ) 
		perror("listen ");
	clilen = sizeof(cliAddr);
	if ( (setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt)) ) < 0 ) {
		perror("setsockopt ");
	}
	/* Blocking call to accept the connection from clients. */
	while (1) {
		printf("waiting/Continue for VALID filename...\n");
		nsd = accept(sd,(struct sockaddr *)&cliAddr,&clilen);

		if ( (pid=fork()) < 0 ) {
			perror("fork ");
			exit(0);
		} else if ( pid == 0 ) {
			/* Client Info */
			printf("Client Details...Connection accepted with following details\n");
			printf("Client PORT Number: %d\n",htons(cliAddr.sin_port));
			printf("Client IP Address : %s\n",inet_ntoa(cliAddr.sin_addr));
			printf("------------------------------------------------------------\n");
			/* As soon as accept the connection Try to get the user credientials to login to remote server */
			#if 1
			len=read(nsd,&u1,sizeof(u1));
			printf("User Credentials read as len:%lu\n",len);
			ret=userAuth(u1.username,u1.password);
			printf("Returned from userAuth function...:%d\n\n",ret);
			ret=1;
			/* Auth success ret value is greated than 1(which is actually userid) --- Auth Fails ret -1 */
			
			if (ret < 0) {
				printf("UserAuthentication Failed...\n");
				sprintf(ch1,"UserAuthentication Failed...");
				len=write(nsd,&ch1,28);
				continue;
			} else {
				printf("UserAuthentication success...\n");
				sprintf(ch1,"UserAuthentication success...");
				len=write(nsd,&ch1,29);
			}
			#endif
			
			printf("Press any key to read the filename from client...\n");
			//getchar();
			memset(ch,'\0',512);

			/* reading filename from client and stores in ch varialbe */
			len=read(nsd,ch,FILENAME_PATH);
			printf("new sockdes: %d --- Filename received is :%s \nAND length:%lu\n-------\n",nsd,ch,len);
			printf("Press any key to start file transfer...\n");
			if ( (seteuid(ret) ) < 0 ) {
				perror("setuid ");
				exit(0);
			} else {
				printf("Current uid is :%d\n",ret);
			}
			//getchar();
			if ( (fp=fopen(ch,"rb")) == NULL ) {
				//perror("Requested file not available...: ");
				sprintf(ch1,"Err|%s",strerror(errno));
				//sprintf(ch1,"FNOTAVAIL");
				len=write(nsd,&ch1,strlen(ch1));
				if ((seteuid(0)) < 0 ) {
					perror("setuid(root) ");
					exit(0);
				}
				continue;
			} else {
				//ONLY TO ALLOCATE MEMORY, WE NEED TO KNOW THE FILESIZE
				if ( stat(ch,&fstat) < 0 ) {
					perror("stat ");
					memset(ch,'\0',512);
					continue;
				} else {
					fileSize=fstat.st_size;
					memset(ch,'\0',512);
				}
				printf("Filesize reported by stat...:%lu\n",fileSize);
				sprintf(ch1,"%lu",fileSize);
				printf("Press key to send FileSize in bytes To client...:%s\n",ch1);
				//getchar();
				/* sending fileinfo to client (rightnow only filesize) */
				len=write(nsd,&ch1,10);
			}
			printf("while loop Ends Here...fileSize:%lu\n",fileSize);
			if ( fileSize <= MAX_PACKET_SIZE ) {
				buf = (char *) malloc (sizeof(char) * fileSize);
				fread(buf,1,fileSize,fp);
				len=write(nsd,buf,fileSize);
				memset(buf,'\0',fileSize);
			} else {
				/* File Content sending CODE */
				tmpFs=0;
				TotalBytesSent=0;
				tosend=MAX_PACKET_SIZE;
				buf = (char *) malloc (sizeof(char) * tosend);
				while (EOFFlag) {
					tmpFs+=MAX_PACKET_SIZE;
					if (tmpFs > fileSize) {
						tmpFs-=MAX_PACKET_SIZE;
						tosend=fileSize-tmpFs;		
						EOFFlag=0;
					}
					fread(buf,1,tosend,fp);
					len=write(nsd,buf,tosend);
					if ( len != tosend) {
						printf("Wrong bytes sent len:%lu and tosend=%d\n",len,tosend);
						//getchar();
					}
					memset(buf,'\0',tosend);
					//printf("Written %lu bytes to socket \n",tmpFs);
				}
				if (buf)
					free(buf);
			printf("While Loop ends succesfully...\n");
			}
			close(nsd);
			fclose(fp);
			EOFFlag=1;
			seteuid(0);
			exit(0);
		} else {
			printf("Parent Process going to continue listining for next client\n");
		}
	}
	close(sd);
	return 0;
}


