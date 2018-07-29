#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
	
    	int  i = 0 ,n = 0, bytes_read;
    	char recvBuff[1024*1024] , sendBuff[1024];
    	struct sockaddr_in serv_addr;
    	char mode[20];  //GET or GETNEW
	int sockfd = 0;

	/*elegxos gia swsth suntaksh ths klhshs tou programmatos client*/
    	if(argc != 4)
    	{
        	printf("\n Usage: %s <-s> <ip of server> <server_port>\n",argv[0]);
        	return 1;
    	}

	/*arxrikopoihsh twn buffers*/
    	memset(recvBuff, 0,sizeof(recvBuff));
    	memset(sendBuff, 0,sizeof(sendBuff));

	//dhmiourgia socket
    	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    	{
        	printf("\n Error : Could not create socket \n");
        	return 1;
    	}

    	memset(&serv_addr, '0', sizeof(serv_addr));

    	serv_addr.sin_family = AF_INET;
    	serv_addr.sin_port = htons(atoi(argv[3]));

	/*lambanoume thn ip tou proxy server apo to argv[2] tou command line*/    	
	if(inet_pton(AF_INET, argv[2], &serv_addr.sin_addr)<=0)
    	{
        	printf("\n inet_pton error occured\n");
        	return 1;
    	}

    	if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    	{
       		printf("\n Error : Connect Failed \n");
       		return 1;
    	}

	while(1)  /* o kathe client mporei na steilei parapanw apo ena aithmata ston server sto session */
	{
		bzero(sendBuff, sizeof(sendBuff));		
		bzero(recvBuff, sizeof(recvBuff));
		int error = 0,a=-1,b=-1,c=-1;
		//elenxos entolhs pou eisagei o xrhsths
		do 
		{
			fflush(stdin);
		
			bzero(sendBuff, sizeof(sendBuff));	
			bzero(recvBuff, sizeof(recvBuff));
		
			printf("\n\nInput format: <COMMAND><SPACE><HOST></><PATH><SPACE><EOF>");
			printf("\nPrompt:");
			fgets(sendBuff, 1024, stdin);
		
			fflush(stdin);
	
			int k = 0, t = 0; /*string indexing*/
			bzero(mode, sizeof(mode));
	
			while(sendBuff[k] != ' ') {mode[k] = sendBuff[k]; k++;} //mode  is collected (GET or GETNEW or END)

			if((a=strcmp(mode,"GET") != 0) && (b=strcmp(mode,"GETNEW") != 0) && (c=strcmp(mode,"END") != 0))
			{
				error = 1;
				printf("\nUnknown command... Type again\n");
			}
			else error = 0;
		}	
		while(error == 1);

		printf("\n You've typed: %s \n", sendBuff);

		//sending query
		int sent = 0, tmpres = 0;
      		while(sent < strlen(sendBuff))
      		{
        		tmpres = send(sockfd, sendBuff+sent, strlen(sendBuff)-sent, 0);
        		if(tmpres == -1)
			{
	          		perror("Can't send query");
	          		exit(1);
	        	}
        		sent += tmpres;
      		}
		
		//an h entolh p eishgage o xrhsths einai "END" tote to programma tou client termatizei
		if(strcmp(mode, "END") == 0)
		{
			close(sockfd); 
			printf("Session closed...Exiting!\n");
			exit(0);
		}

		bzero(sendBuff, sizeof(sendBuff));
		//receiving html code from proxy server
    		do
    		{
		/*
		se kathe epanalhpsh o client kanei receive apo ton proxy dedomena megethous sizeof(recvBuff) mexri h recv na epistrepsei
		bytes mikroterou megethous apo to sizeof(recvBuff), pou shmainei oti exei diabasei to teleutaio tmhma tou html kwdika
		*/
        		bzero(recvBuff, sizeof(recvBuff));
        		bytes_read = recv(sockfd, recvBuff,sizeof(recvBuff), 0);
		
			if ( bytes_read > 0 )
			{
        			fprintf(stdout,"%s", recvBuff);
        		}
		}
    		while ( bytes_read == sizeof(recvBuff) );

	}   //endOfWhile(1)

	close(sockfd);  //kleinoume to socket epikoinwnias me ton server
	
    return 0;
}
