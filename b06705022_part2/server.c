#include<stdio.h>
#include<string.h>	//strlen
#include<stdlib.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write
#include<stdbool.h> //use bool in c

#include<pthread.h> //for threading , link with lpthread

void *connection_handler(void *);
void get_ip(int descriptor, char ip[20]);

//user data
int user_cnt = 0;
char name[100][20];
int user_bal[100];
//online data
int online_cnt = 0;
char online_ip[100][20];
int online_port[100];
bool isOnline[100];


int main(int argc , char *argv[])
{
	int socket_desc , new_socket , c , *new_sock;
	struct sockaddr_in server , client;
	char *message;

	//reset user data
	memset(name, 0, 2000);
	memset(user_bal, 0, 100*sizeof(int));
	//reset online data
	memset(online_ip, 0, 2000);
	memset(online_port, 0, 100*sizeof(int));
	memset(isOnline, false, 100 * sizeof(bool));
	
	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 8888 );
	
	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("bind failed");
		return 1;
	}
	
	//Listen
	listen(socket_desc , 3);
	
	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
		puts("Connection accepted");

		pthread_t sniffer_thread;
		new_sock = malloc(1);
		*new_sock = new_socket;
		
		if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
		{
			perror("could not create thread");
			return 2;
		}
		
		//Now join the thread , so that we dont terminate before the thread
		//pthread_join( sniffer_thread , NULL);
		puts("Handler assigned");
	}
	
	if (new_socket<0)
	{
		perror("accept failed");
		return 1;
	}
	
	return 0;
}

void *connection_handler(void *socket_desc)
{
	//Get the socket descriptor
	int sock = *(int*)socket_desc;
	int read_size;
	int client_idx = -1;
	char message[500] , client_message[2000];

	//Reply to new client
    strcpy (message, "Connection accepted!\n");
    write(sock , message , strlen(message));

	//get client ip
	char client_ip[20];
	memset(client_ip, 0, 20);
	get_ip(sock, client_ip);
	
	//start receiving message from client
	while(1)
	{
		//reset message
        memset(client_message, 0, 2000); 
		memset(message, 0, 500);
        //receive message
        read_size = recv(sock , client_message , 2000 , 0);

		char temp[2000];
		char *ptr = temp;
		strcpy(temp, client_message);

        if (read_size == 0)
        {
            puts("Client disconnected");
            fflush(stdout);
            break;
        }
        if (read_size == -1)
        {
            perror("recv failed");
            break;
        }
		if (strncmp(ptr, "REGISTER#", 9) == 0)
		{
			ptr = strtok(temp, "#");
			/* <Register#name__\0> */
			//  ^ptr
			ptr = strtok(NULL, "\n\r");
			//           ^ptr 
			strtok(ptr, "\n\r");
			//replace last word
			//               ^ptr
			//print register name
			printf("Register: %s\n", ptr);

			for(int i = 0; i < user_cnt; i++)
			{
				if(strcmp(ptr, name[i]) == 0)
				{
					strcpy(message, "210 FAIL\n");
					write(sock, message, strlen(message));
					ptr = NULL;//flag 1
					break;
				}
			}
			if(ptr != NULL)//flag 1
			{
				/* REGISTER SUCCESS */
				strcpy(name[user_cnt], ptr);
				user_cnt++;

				strcpy(message, "100 OK\n");
				write(sock, message, strlen(message));
			}
			continue;
		}
		if (strcmp(ptr, "List\n") == 0)
		{
			if(client_idx == -1)
			{
				strcpy(message, "220 AUTH_FAIL\n");
				write(sock, message, strlen(message));
			}
			else
			{
				ptr = &message[0];
				/* List: <accountBalance><CRLF> */
				sprintf(message, "%d\n", user_bal[client_idx]);
				ptr = strchr(ptr, '\0');

				/* List: <number of accounts online><CRLF> */
				sprintf(ptr, "number of accounts online: %d\n", online_cnt);
				ptr = strchr(ptr, '\0');

				for(int i = 0; i < user_cnt; i++)
				{
					if(isOnline[i])
					{
						/* List: <userAccount>#<IP>#<Port_num><CRLF>*/
						sprintf(ptr, "%s#%s#%d\n", name[i], online_ip[i], online_port[i]);
						ptr = strchr(ptr, '\0');
					}
				}
				write(sock, message, strlen(message));
			}
			continue;
		}
		if (strcmp(ptr, "Exit\n") == 0)
		{
			if(client_idx != -1)
			{
				online_port[client_idx] = 0;
				isOnline[client_idx] = false;
				online_cnt--;
				client_idx = -1;
			}
			strcpy(message, "Bye\n");
			write(sock, message, strlen(message));
			continue;
		}
        /* else (login): name#port_num */
		if(ptr == NULL)
			continue;
        ptr = strtok(temp, "#");
		
			int idx = -1, port = -1;
			char new_name[20];
			bool success = false;
			memset(new_name, 0, 20);
			strcpy(new_name, ptr);
			ptr = strtok(NULL, "\n\r");

			for(int i = 0; i < user_cnt; i++)
			{
				if(strcmp(new_name, name[i]) == 0)
				{
					puts("user find");
					strcpy(message, name[i]);
					idx = i;
					success = true;
					if(online_port[i] != 0)
						success = false;
					break;
				}
			}
			if(client_idx != -1)
				success = false;
			if(success)
			{
				strcpy(online_ip[idx], client_ip);
				online_port[idx] = atoi(ptr);
				isOnline[idx] = true;
				client_idx = idx;
				online_cnt++;
				ptr = &message[0];
				/* List: <accountBalance><CRLF> */
				sprintf(message, "%d\n", user_bal[client_idx]);
				ptr = strchr(ptr, '\0');

				/* List: <number of accounts online><CRLF> */
				sprintf(ptr, "number of accounts online: %d\n", online_cnt);
				ptr = strchr(ptr, '\0');

				for(int i = 0; i < user_cnt; i++)
				{
					if(isOnline[i])
					{
						/* List: <userAccount>#<IP>#<Port_num><CRLF>*/
						sprintf(ptr, "%s#%s#%d\n", name[i], online_ip[i], online_port[i]);
						ptr = strchr(ptr, '\0');
					}
				}
				write(sock, message, strlen(message));
			}
			else
			{
				strcpy(message, "220 AUTH_FAIL\n");
				write(sock, message, strlen(message));
			}
			continue;
		
		/* else default: */
        //Send the message back to client
		client_message[read_size] = '\0';
		strcpy(message, client_message);
        write(sock, message, strlen(message));
        printf("%s", message);

		
	}
	//Free the socket pointer
	free(socket_desc);
	
	return 0;
}

/*Obtain the client ip through descriptor*/
void get_ip(int descriptor, char ip[20])
{
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    // The function returns the client ip associated with descriptor
    getpeername(descriptor, (struct sockaddr *)&addr, &addr_size);
    strcpy(ip, inet_ntoa(addr.sin_addr)); // convert the given ip into bytes to string
}