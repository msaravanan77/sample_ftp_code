#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netinet/tcp.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>

#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>

#define MAX_PACKET_SIZE 1024
#define FILENAME_PATH	512
#define SERVER_PORT 5000

int userAuth(char *uname,char *password);

