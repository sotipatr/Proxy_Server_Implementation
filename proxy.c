#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <resolv.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#define MAXBUF  1024*1024

int listenfd = 0;
/*sunarthsh gia signal handling Ctrl-C*/
void catch_int(int sig_num)
{
	signal(SIGINT, catch_int);
	close(listenfd);
	exit(0);
}

int main(int Count, char *Strings[])
{   
	/*signal handling*/
	signal(SIGINT, catch_int);

	/*socket epikoinwnias me client*/
	int connfd = 0;
	struct sockaddr_in serv_addr;
	char recvBuff[1024];
	char temp_c[1024];
	int bytes_rcv = 0;
	char location_buffer[50];
    	char html_content[1024*1024];

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
    	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
    	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    	serv_addr.sin_port = htons(atoi(Strings[2]));
    
	bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	listen(listenfd, 10);

	/*o server dexetai kainouria aithmata*/
	while(1)
    	{
		printf("proxy is running...\n");
		/*apodoxh aithmatwn mesw ths accept*/
		connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

		if (connfd < 0) 
    		{
        		perror("ERROR on accept");
        		exit(1);
    		}
		printf("Connected to a new client...\n");
		
		char mode[20];  /*GET or GETNEW*/
		
		/*o server dexetai parapanw apo ena aithma apo kathe client mexri na labei mhnuma telous ("END")*/
		do
    		{
		
			bzero(html_content, sizeof(html_content));
    			bzero(recvBuff, sizeof(recvBuff));

        		bytes_rcv = recv(connfd, recvBuff, sizeof(recvBuff), 0);
			strcpy(temp_c, recvBuff);
			printf("\n You've received: %s \n", temp_c);

    			int sockfd, bytes_read;
    			struct sockaddr_in *dest;
    			char buffer[MAXBUF]; /*apothhkeuei olh tn apanthsh apo ton web server*/
    			char host[200];  /*to string onoma tou web server*/
    			char save_location[50];  /*to path sto pc topika p apothhkeuetai h html selida*/

    			FILE* saved_page;   /*dhmiourgia file descriptor gia to html arxeio pou tha apothhkeutei h selida ston proxy*/

			char url[200];  //the url without the host
			int k = 0, t = 0; /*string indexing*/
			bzero(mode, sizeof(mode));
			while(temp_c[k] != ' ') {mode[k] = temp_c[k]; k++;} //mode  is collected
			printf("mode: %s\n", mode);
			k++; //we point to the beginning of the next substring
			bzero(host, sizeof(host));
			while(temp_c[k] != '/') {host[t] = temp_c[k]; k++; t++;} //host is collected
			printf("host: %s\n", host);
			k++; //we point to the beginning of the next substring
			t = 0;
			bzero(url, sizeof(url));
			while(temp_c[k] != EOF) {url[t] = temp_c[k]; k++; t++;} //url is collected
			printf("url: %s\n", url);

		
			bzero(location_buffer, sizeof(location_buffer));
    			strcpy(location_buffer, url);
    			int i = 0;

			if(strcmp(mode,"END")==0)  
				printf("END BYE BYE....!\n");

    			/*replace '/' character with '_' in the requested html file path*/
    			while(i < strlen(location_buffer))
			{

    				if(location_buffer[i] == '/') location_buffer[i] = '_';
        			i++;
    			}
			sprintf(save_location, "cache/%s_%s", host, location_buffer);

    			printf("save location: %s\n",save_location);

			int found = 0;
			/*an h entolh tou client einai "GET..." tote o proxy psaxnei gia to arxeio ston disko kai an 
			  to brei to apothhkeuei sto html_content, an oxi to xeirizetai h parakatw if */
			if(strcmp(mode,"GET")==0)  
			{
				printf("GET!\n");
				if((saved_page = fopen((const char *)save_location,"r")) != NULL )  
				{
					found = 1;
					fread(html_content,1,sizeof(html_content),saved_page);
					fclose(saved_page);
				}
				else printf("file not found \n");
			}
		
			/*an h entolh tou client einai "GETNEW ..." tote o proxy epikoinwnei me ton antistoixo 
			  web server kai apothhkeuei thn html selida sto html_content */
			if((strcmp(mode,"GETNEW")==0) || found == 0)
			{ 
				printf("GETNEW!\n");

				/*dhmiourgia socket epikoinwnias me web server*/
    				if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {perror("Socket creation error"); return 1;}
        
    				struct hostent *hent;
      				int iplen = 100; 
      				char *ip = (char *)malloc(iplen+1);
      				memset(ip, 0, iplen+1);
				printf("host: %s\n", host);
				int hostOK = 1;
				/*lambanetai h IP tou web server kai an den einai eglurh proxwraei se epomenh epanalhpsh*/
      				if((hent = gethostbyname(host)) == NULL)
      				{
        				hostOK = 0;
					herror("Can't get IP");
      				}
				/*an h IP 'h to DNS name pou edwse o client einai egkuro proxwraei sto connection*/
				if(hostOK == 1)
				{
      					inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, iplen);

    					printf("The ip is: %s\n", ip);
    					/*---Initialize server address/port struct---*/
    					dest = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
    					memset(dest, 0, sizeof(struct sockaddr_in));
    					dest->sin_family = AF_INET;
    					int tmpres;
    					tmpres = inet_pton(AF_INET, ip, (void *)(&(dest->sin_addr.s_addr)));
      		
    					dest->sin_port = htons(80); /*default HTTP Server port */
       
    					/*---Connect to server---*/
    					connect(sockfd, (struct sockaddr*)dest, sizeof(struct sockaddr));
					
					/*HTTP request*/
					sprintf(buffer, "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: HTMLGET 1.0\r\n\r\n", url, host);
	
					/*sending http request*/      					
					int sent = 0;
      					while(sent < strlen(buffer))
      					{
        					tmpres = send(sockfd, buffer+sent, strlen(buffer)-sent, 0);
        					if(tmpres == -1)
						{
          						perror("Can't send query");
							close(sockfd);
							close(connfd);
							close(listenfd);
          						exit(1);
        					}
        					sent += tmpres;
      					}

    					/*---While there's data, receive it---*/
    					char temp[1024*1024];
					bzero(temp, sizeof(temp));

    					do
    					{
        					bzero(buffer, sizeof(buffer));
        					bytes_read = recv(sockfd, buffer, sizeof(buffer), 0);
        					if ( bytes_read > 0 )
							strcat(temp, buffer);
    					}
    					while ( bytes_read > 0 );
	
					bzero(html_content, sizeof(html_content));
					
					/*briskei to shmeio pou ksekinaei o html kadikas*/
    					i = 0;
    					while(temp[i] != '<') i++;

					/*apomonwnei ton html kwdika apo th sunolikh apanthsh tou web server*/
    					for(k=0; k<strlen(temp)-i; k++) html_content[k] = temp[k+i];
					saved_page = fopen((const char *)save_location,"w+");

					/*apothhkeush ths html selidas ston disko*/
    					fprintf(saved_page,"%s", html_content);
					/*---Clean up---*/
					close(sockfd);  /*kleisimo socket epikoinwnias me ton web server*/
					free(dest);
					fclose(saved_page);	
				} //if(hostOK)
				/*an h IP den einai egkurh o client enhmerwnetai*/
				else 
				{
					printf("IP NOT VALID!\n");
					strcpy(html_content, "Ip not valid!!!\n");
				}
    				
    		
			}  //endOfIf-getnew
			int sent = 0, tmpres = 0;
			/*an o client den exei steilei mhnuma telous tou stelnetai to periexomeno tou html_content*/
			if(strcmp(mode, "END") !=0)
			{
				/*oso ta bytes pou exoun stalthei einai ligotera apo ta bytes dedomenwn tou html_content
				  ginontai send twn periexomenwn tou html_content ston client*/
      				while(sent < strlen(html_content))
      				{
        				tmpres = send(connfd, html_content+sent, strlen(html_content)-sent, 0);
  					if(tmpres == -1)
					{
          					perror("Can't send query");
						close(connfd);
          					exit(1);
        				}
        				sent += tmpres;
      				}
			}//end if mode==end

		}  //endOfnestedDo-while
		while(strcmp(mode, "END") != 0);

		close(connfd);  /*kleisimo socket epikoinwnias me ton client*/
	}  //endOfWhile
    	return 0;
}  //endOfmain

