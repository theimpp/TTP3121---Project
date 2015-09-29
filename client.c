#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <signal.h>
#include <ctype.h>          
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <dirent.h>
#define BUFSIZE 1024

void error(const char *msg){
    perror(msg);
    exit(0);
}

void createFile(int sockfd){	//Creating file on client-site
  	printf("Creating a file...");
	
	//Setting directory
	char content[256];
	char dir[256] = "/home/";
	char hostname[256];
	//gethostname(hostname, 255);
	char file[256] = "izaki/Client/";
	//strcat(dir, hostname);
	strcat(dir, file);
	printf("\nPath: %s", dir);
	
	//Create directory if it does not exist	
	struct stat st = {0};
	if(stat(dir, &st) == -1){
	  mkdir(dir, 0700);
	}
	
	//Create file name
	char filename[256];
	printf("\nPlease enter you file name: ");
	gets(filename);//
	
	if(filename != NULL){
		strcat(dir, filename);
		printf("The File location: %s\n", dir);
		//Create file
		FILE *fp;
		fp = fopen(dir, "w+");
		if(fp == NULL){
		  printf("\nERROR: File cannot be created, Sorry\n");
		  perror("fopen");	
		  exit(0);	
		}
		else{	//Client insert content
		  printf("Please insert you content here: ");
		  gets(content);
		  printf("Content: %s", content);
		 
		  fprintf(fp, "%s", content);	//Write content into the file
		  fclose(fp);
		  printf("\nFile have created successfully!");
		}
	}
	else{
		printf("\nERROR: Filename cannot be NULL");		
		printf("\nERROR: Please try it again");
		exit(0);
	}
}

void downloadFile(int sockfd){	//Client download file from Server
	printf("Downloading file from Server!!! ");
	
	int n;
	int buflen;

	//Setting directory
	char revBuff[256];
	char dir[256] = "/home/";
	char hostname[256];
	//gethostname(hostname, 255);
	char file[256] = "izaki/Client/";
	//strcat(dir, hostname);
	strcat(dir, file);
	printf("\nPath: %s", dir);
	
	//Create directory if it does not exist
	struct stat st = {0};
	if(stat(dir, &st) == -1){
	  mkdir(dir, 0700);
	}

	//Getting available file from Server
	char tempo[256];
	bzero(tempo,256);
	n = read(sockfd, (char*)&buflen, sizeof(buflen));
	if (n < 0) error("ERROR reading from socket");
	buflen = htonl(buflen);
	n = read(sockfd,tempo,buflen);
	if (n < 0) error("ERROR reading from socket");
	printf("\nAvailable file: \n");
	printf("%s", tempo);

	printf("Please enter the file name that you want to download: ");
	char selectFile[256];
	bzero(selectFile,256);
	gets(selectFile);
    	char input[256];
	
	//Sending file name that Client wants to download to Server
	int datalen = strlen(selectFile);
	int tmp = htonl(datalen);
	n = write(sockfd, (char*)&tmp, sizeof(tmp));
	if(n < 0) error("ERROR writing to socket");
	n = write(sockfd,selectFile,datalen);
	if (n < 0) error("ERROR writing to socket");
	
	char filename[256];
	printf("Save you file name as: ");
	fgets(filename, 256, stdin);

	if(filename != NULL){
		strcat(dir, filename);	//Concatenate directory and filename
		printf("File location: %s", dir);

		FILE *fr = fopen(dir, "ab");
		if(fr == NULL){
		  printf("You file cannot be opened");
		  perror("fopen");
		  exit(0);
		}
		else{	//Receiving file from Server 
		  bzero(revBuff, 256);
		  int fr_block_sz = 0;
		  while((fr_block_sz = recv(sockfd, revBuff, 256, 0)) > 0){
		  	int write_sz = fwrite(revBuff, sizeof(char), fr_block_sz, fr);
			if(write_sz < fr_block_sz){
			  error("File write failed on server.\n");
			}
			bzero(revBuff, 256);
			if(fr_block_sz == 0 || fr_block_sz != 256){
			  break;			
			}
		  }
		  printf("\nFile downloaded successfully");
		  fclose(fr);
		}
	}
	else{
		printf("\nERROR: Filename cannot be NULL");		
		printf("\nERROR: Please try it again");
		exit(0);
	}
}

void sendFile(int sockfd){	//Client send file to Server

	/* Send File to Server */
        char* fs_name = NULL;
        char sdbuf[BUFSIZE]; 
	char* location = NULL;
	char buffer[BUFSIZE+1];
	printf("[Client] Select the File with Full File Path: [example: /home/izaki/...]\n");
	gets(buffer);
	fs_name = buffer;

	//char* name = malloc(strlen(fs_name)+strlen(filename) + 2);
    	//sprintf(name,"%s%s", fs_name, filename);
	
        FILE *fs = fopen(fs_name, "r");

        if(fs == NULL)
        {
            printf("ERROR: File %s not found. Try again!!!!!\n", fs_name);
	    //exit(1);
        }
	else{
	
	printf("[Client] Path Successful load, Enter File name:\n");
	gets(buffer);
	send (sockfd, buffer, BUFSIZE,0);
        bzero(sdbuf, BUFSIZE); 
        int fs_block_sz; 
        while((fs_block_sz = fread(sdbuf, sizeof(char), BUFSIZE, fs)) > 0)
        {
            if(send(sockfd, sdbuf, fs_block_sz, 0) < 0)
            {
                fprintf(stderr, "ERROR: Failed to send file %s. (errno = )\n", fs_name);
                exit(1);
            }
            bzero(sdbuf, BUFSIZE);
        }
	printf("[Client] Sending %s to the Server... \n", fs_name);
        printf("Ok File %s from Client was Sent!\nFile are saved in Server", fs_name);
	}
		
}

void deleteFile(int sockfd){	//Deleting file on client-site
	printf("Deleting a file...");
	
	//Setting directory
	char content[256];
	char dir[256] = "/home/";
	char hostname[256];
	//gethostname(hostname, 255);
	char file[256] = "izaki/Client/";
	//strcat(dir, hostname);
	strcat(dir, file);
	printf("\nPath: %s", dir);
	
	//Create directory if it does not exist	
	struct stat st = {0};
	if(stat(dir, &st) == -1){
	  mkdir(dir, 0700);
	}

	//Printing files that is available from the directory
	printf("\nAvailable file: \n");
	DIR *directory;
	struct dirent *ent;
	if((directory = opendir(dir)) != NULL){
	  while((ent = readdir(directory)) != NULL){
		printf("%s", ent->d_name);
	  }
	  closedir(directory);
	}
	else{
	  perror("ERROR");
	  exit(0);
	}

	//Getting file name to be deleted
	char filename[256];
	printf("\nPlease enter the file name that you want to delete: ");
	fgets(filename, 256, stdin);

	
	if(filename != NULL){

		strcat(dir, filename);
		FILE *fp;
		
		//Check if file available
		fp = fopen(dir, "r");
		if(fp == NULL){
		  printf("\nERROR: File cannot be created");
		  perror("fopen");	
		  exit(0);	
		}
		else{	//Deleting file
		  int status = remove(dir);
		  if(status == 0){
			printf("\nFile deleted successfully!");
			fclose(fp);
		  }else{
			printf("\nERROR: unable to delete the file");
			exit(0);
		  }
		}
	}
}

int main(int argc, char *argv[])	//Connecting to Server (SOCKET)
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;


    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    printf("\n\nYou are now connected to the Server!");



    int count = 0;
    while(count == 0){	//Getting Client's choice
	int proceed = 0;
	printf("\n\n1.Create 2.Download 3.Send 4.Delete 5.Exit : ");
	printf("\nPlease select your choice: ");
	bzero(buffer,256);
	fgets(buffer,255,stdin);
    	char input[256];
	strcpy(input, buffer);
	
	//Sending Client's choice to Server
	int datalen = strlen(buffer);
	int tmp = htonl(datalen);
	n = write(sockfd, (char*)&tmp, sizeof(tmp));
	if(n < 0) error("ERROR writing to socket");
	n = write(sockfd,buffer,datalen);
	if (n < 0) error("ERROR writing to socket");
	

	if((strcmp(input, "1\n")) == 0){	//Create file on client-site
	   createFile(sockfd);
	   count = 0;
	}
	else if((strcmp(input, "2\n")) == 0){	//Client download file from Server
	   downloadFile(sockfd);
	   count = 0;
	}
	else if((strcmp(input, "3\n")) == 0){	//Client send file to Server
	   sendFile(sockfd);
	   count = 0;
	}
	else if((strcmp(input, "4\n")) == 0){	//Delete file on client-site
	   deleteFile(sockfd);
	   count = 0;
	}
	else if((strcmp(input, "5\n")) == 0){	//Client disconnect from Server
	   count = 1;
	   proceed = 1;
	}
	else{
	   printf("\nWrong input, please try again.");	//Invalid input from Client
	   count = 0;
	}
    }
	
    close(sockfd);
    printf("\nYou have disconnected from the Server.\n\n");
    return 0;
}


