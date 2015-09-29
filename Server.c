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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <dirent.h>
#define BUFSIZE 1024

void dostuff(int); 
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) /* Creating Socket for multiple Client */
{

     int num;
     int sin_size; 

     int sockfd, newsockfd, portno, pid;

     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     printf("\nServer address: %s \n" , inet_ntoa(serv_addr.sin_addr));
     printf("\nServer Port Number: %d \n", ntohs(serv_addr.sin_port));
     printf("Server Waiting for Connection....\n");
	
     while (1) {
         newsockfd = accept(sockfd, 
               (struct sockaddr *) &cli_addr, &clilen);
	
	 printf("Server: Connection from %s \n", inet_ntoa(cli_addr.sin_addr));
         if (newsockfd < 0) 
             error("ERROR on accept");
         pid = fork();
         if (pid < 0)
             error("ERROR on fork");
         if (pid == 0)  {
             close(sockfd);
	     printf("\nClient %s has connected to the Server.\n", inet_ntoa(cli_addr.sin_addr) );
             dostuff(newsockfd);
             exit(0);
         }
         else close(newsockfd);
     } /* end of while */
     close(sockfd);
     return 0; /* we never get here */
}

/******** DOSTUFF() *********************
 There is a separate instance of this function 
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/
void dostuff (int sock)
{
   //printf("Client has connected to the Server.\n");
   int count = 0;
   int buflen;
   while(count == 0){

	int n;
	char buffer[256];

	//Get Client's choice
	bzero(buffer,256);
	n = read(sock, (char*)&buflen, sizeof(buflen));
	if (n < 0) error("ERROR reading from socket");
	buflen = htonl(buflen);
	n = read(sock,buffer,buflen);
	if (n < 0) error("ERROR reading from socket");

	printf("\nClient's selection: %s\n",buffer);
	
	//Function for for different choices
	if(buffer != NULL){
	if((strcmp(buffer, "1\n")) == 0){	//Client perform create file on client-site
		count = 0;
	}
	else if((strcmp(buffer, "2\n")) == 0){	//Client request to download file from server
	
		printf("Sending file to Client...");
		char buff[256];
		int n;
		
		//Setting directory
		char dir[256] = "/home/";
		char hostname[256];
		//gethostname(hostname, 255);
		char file[256] = "izaki/Server/";
		//strcat(dir, hostname);
		strcat(dir, file);
		printf("\nPath: %s", dir);
			
		//Retrieving files from directory
		char tempo[256];
		printf("\nAvailable file: ");
		DIR *directory;
		struct dirent *ent;
		if((directory = opendir(dir)) != NULL){
		  while((ent = readdir(directory)) != NULL){
			printf("\n%s", ent->d_name);
			strcat(tempo, ent->d_name);
			strcat(tempo, "\n");
		  }
		  closedir(directory);
		}
		else{
		  perror("ERROR");
		  exit(0);
		}

		//Sending files available to Client
		int datalen = strlen(tempo);
		int tmp = htonl(datalen);
		n = write(sock, (char*)&tmp, sizeof(tmp));
		if(n < 0) error("ERROR writing to socket");
		n = write(sock,tempo,datalen);
		if (n < 0) error("ERROR writing to socket");

		//Storing filename that Client wanted to download
		char fileRev[256];
		bzero(fileRev,256);
		n = read(sock, (char*)&buflen, sizeof(buflen));
		if (n < 0) error("ERROR reading from socket");
		buflen = htonl(buflen);
		n = read(sock,fileRev,buflen);
		if (n < 0) error("ERROR reading from socket");

		char split[2] = "\n";
		strtok(fileRev, split);
		printf("\nSending file %s to Client... \n", fileRev);
		
		//Sending files to Client
		if(fileRev != NULL){
			strcat(dir, fileRev);
			FILE *fs = fopen(dir, "rb");
			if(fs == NULL){
			  printf("ERROR: File not found.\n");
			  perror("fopen");
			  break;
			}
			else{
			  //Writing file to Client
			  bzero(buff, 256);
			  int fs_block_sz;
			  while((fs_block_sz = fread(buff, sizeof(char), 256, fs)) > 0){
			    if(send(sock, buff, fs_block_sz, 0) < 0){
				fprintf(stderr, "ERROR: Failed to send file. %d", errno);
				break;
			    }
			    bzero(buff, 256);
			  }
			  printf("\nFile sent successfully!\n");
		 	  fclose(fs);
			}
		}
		else{
			printf("\nERROR: Filename cannot be NULL");		
			printf("\nERROR: Please try again later");
			//exit(0);
		}
		count = 0;
	}
	else if((strcmp(buffer, "3\n")) == 0){ //Client sending file to Server
		
		printf("Server: Prepare to Receive File....\n");
 		/*Receive File from Client */
        	char* fr_name = "/home/izaki/Server/";
		recv(sock, buffer, BUFSIZE, 0);
	
		char* name = malloc(strlen(fr_name)+strlen(buffer) + 2);
    		sprintf(name,"%s%s", fr_name, buffer);
		//fr_name = buffer;
       		FILE *fr = fopen(name, "a");

        	if(fr == NULL)
		{
			//strcpy(buffer, "0");
			//send(new_sockfd, buffer, BUFSIZE, 0);
            		printf("File %s Cannot be opened file on server.\n", fr_name);
			
		}
       		else
        	{
            		bzero(buffer, BUFSIZE); 
            		int fr_block_sz = 0;
            		while((fr_block_sz = recv(sock, buffer, BUFSIZE, 0)) > 0) 
            		{
                		int write_sz = fwrite(buffer, sizeof(char), fr_block_sz, fr);
                		if(write_sz < fr_block_sz)
                		{
                    			error("File write failed on server.\n");
                		}

                		bzero(buffer, BUFSIZE);
                			if (fr_block_sz == 0 || fr_block_sz != 512) 
                				{
                   				 	break;
                				}//end if
            	 	 }//end while
            	if(fr_block_sz < 0)
            	{
                	if (errno == EAGAIN)
                		{
                    			printf("recv() timed out.\n");
                		}// end if
                	else
                		{
                   		     fprintf(stderr, "recv() failed due to errno = %d\n", errno);
                    			exit(1);
                		}//end else
            	}//end if

            		printf("Ok received File from client via socket %d \n" ,sock);
            		fclose(fr); 
        	}// end big else

	//received file end....
          //close(new_sockfd);
          printf("[Server] Server will wait now...\n");

		count = 0;
	}
	else if((strcmp(buffer, "4\n")) == 0){	//Client perform delete function on client-site
		count = 0;
	}
	else if((strcmp(buffer, "5\n")) == 0){	//Client request to disconnect from Server
		
		printf("Client is disconnecting from the Server... \n");
		count = 1;
	}
	else{	//Invalid input 
		printf("\nClient inserted wrong input.\n\n");
		count = 0;
		break;
	}
	}

   }
}
