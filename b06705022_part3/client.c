#include <stdio.h>
#include<stdlib.h>	//strlen
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>	//function inet_addr : convert IP address
#include <unistd.h> //read
#include<stdbool.h> //use bool in c

#include<pthread.h> //for threading , link with lpthread


void *recv_connection_handler(void *);
void *p2p_connection_handler(void *);
void *sender_connection_handler(void *);

char p2p_message[1024];
char my_name[500];



char name[10];

int main()//int argc , char *argv[])
{
	int socket_desc;
	struct sockaddr_in server;
	char server_reply[1024];

	memset(my_name, 0, sizeof(my_name));
	
	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
		
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons( 8888 );

	//Connect to server
	if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("connect error");
		return 1;
	}

	//Receive and print a reply from the server
	read(socket_desc, server_reply, 1024);
	printf("%s", server_reply);
	memset(server_reply, 0, sizeof(server_reply));
	bool isLogin = 0;
	while(1)
	{
		int my_balance = 100;
		char cmd[20], message[20]; // command only as q/1/2/3
			printf("You can:\n");
			printf("(q) Quit and do nothing.\n");
		if(!isLogin)
		{
			printf("(1) Register for Micropayment System.\n");
			printf("(2) Log in to system.\n");
		}
		if(isLogin)
		{
			printf("(3) See the latest online client list\n");
			printf("(4) Pay\n");
			printf("(5) Receive Payment\n");
		}
		printf("--- Type one of the keys in parentheses --- ");

		scanf("%s", cmd);
		printf("\n");
		if (strcmp(cmd, "q") == 0) 
		{
			char *msg = "Exit\n";
			send(socket_desc, msg, strlen(msg), 0);
			read(socket_desc, server_reply, 1024);
			break;
		} 
		else if (strcmp(cmd, "1") == 0 && !isLogin)
		{
			char message[20];
			printf("Enter your register name(2 to 10 letters).\n");
			printf("New username: ");
			scanf("%s", name);
			//message: "REGISTER#name\n"
			strcpy(message, "REGISTER#");
			strcat(message, name);
			strcat(message, "\n");
			printf("\n");
			send(socket_desc, message, strlen(message), 0);
			read(socket_desc, server_reply, 1024);
			puts(server_reply);
		}
		else if (strcmp(cmd, "2") == 0 && !isLogin)
		{
			isLogin = true;
			char message[20], port[10];
			printf("Enter your username and port number.\n");
			printf("login username: ");
			scanf("%s", message);
			strcpy(my_name, message);
			printf("port number: ");
			scanf("%s", port);
			//message: "name#1234\n"
			strcat(message, "#");
			strcat(message, port);
			strcat(message, "\n");
			printf("\n");
			send(socket_desc, message, strlen(message), 0);
			read(socket_desc, server_reply, 1024);
			puts(server_reply);
			if(strcmp(server_reply, "user not found") == 0)
			{
				isLogin = false;
			}
			else//login success
			{
				pthread_t recv_thread;
				int new_socket = atoi(port), *recv_sock;
				recv_sock = malloc(1);
				*recv_sock = new_socket;
				if( pthread_create( &recv_thread , NULL ,  recv_connection_handler , (void*) recv_sock) < 0)
				{
					perror("could not create thread");
					return 1;
				}
			}
		}
		else if (strcmp(cmd, "3") == 0 && isLogin)
		{
			//message: "List\n"
			strcpy(message, "List\n");
			send(socket_desc, message, strlen(message), 0);
			read(socket_desc, server_reply, 1024);
			puts(server_reply);
			
		}
		else if (strcmp(cmd, "4") == 0 && isLogin)
		{
			//pay
			strcpy(message, "List\n");
			send(socket_desc, message, strlen(message), 0);
			read(socket_desc, server_reply, 1024);
			puts(server_reply);
			char* ptr = strtok(server_reply, "\n");
			my_balance = atoi(ptr);
			char pay_name[20], pay_IP[20], pay_port[20], pay_amount[20];
			printf("Enter payee Name:");
			scanf("%s", pay_name);
			printf("Enter payee IP:");
			scanf("%s", pay_IP);
			printf("Enter payee port:");
			scanf("%s", pay_port);
			printf("Enter pay amount:");
			scanf("%s", pay_amount);
			if(atoi(pay_amount) > my_balance)
				puts("Your payment exceed account balance\n");
			else
			{
				char p2p_message[500];
				strcpy(p2p_message, pay_IP);
				strcat(p2p_message, "?");
				strcat(p2p_message, pay_port);
				strcat(p2p_message, "?");
				strcat(p2p_message, my_name);
				strcat(p2p_message, "#");
				strcat(p2p_message, pay_amount);
				strcat(p2p_message, "#");
				strcat(p2p_message, pay_name);
				pthread_t my_thread;
				int p2p_socket_desc;
				if(pthread_create(&my_thread, NULL, sender_connection_handler, (void*) p2p_message) < 0)
				{
					perror("fail123");
					return 1;
				}
			}
		}
		else if (strcmp(cmd, "5") == 0 && isLogin)
		{
			//receive pay
			strcpy(message, "List\n");
			send(socket_desc, message, strlen(message), 0);
			read(socket_desc, server_reply, 1024);
			send(socket_desc, p2p_message, strlen(p2p_message), 0);
			memset(server_reply, 0, sizeof(server_reply));
			memset(p2p_message, 0, sizeof(p2p_message));
		}
		else /* default: */
		{
			printf("command not found: %s Please re-enter.\n\n", cmd);
			continue;
		}
		memset(server_reply, 0, sizeof(server_reply));
	}
	close(socket_desc);
	return 0;
}

void *recv_connection_handler(void *recv_sock)
{
	//work like listening server 
	int portnum = *(int*)recv_sock;
	int sock_desc, new_socket, len, *new_sock;
	struct sockaddr_in payee;//server
	struct sockaddr_in payer;//client
	
	//Create socket
	sock_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (sock_desc == -1)
	{
		printf("Could not create socket");
	}

	//Prepare the sockaddr_in structure
	payee.sin_family = AF_INET;
	payee.sin_addr.s_addr = INADDR_ANY;
	payee.sin_port = htons( portnum );

	//Bind
	if( bind(sock_desc, (struct sockaddr *)&payee, sizeof(payee)) < 0)
	{
		puts("in recv_handler bind failed");
	}

	//listen
	listen(sock_desc, 3);

	//Accept and incoming connection
	puts("waiting for payer...");
	len = sizeof(struct sockaddr_in);
	pthread_t p2p_thread;
	while( (new_socket = accept(sock_desc, (struct sockaddr *)&payer, (socklen_t*)&len)))
	{
		puts("p2p accepted");

		new_sock = malloc(1);
		*new_sock = new_socket;

		if( pthread_create( &p2p_thread, NULL, p2p_connection_handler, (void*) new_sock))
		{
			perror("in recv_handler could not create thread");
		}
	}
	if (new_socket < 0)
	{
		perror("in recv handler accept failed");
	}
	return 0;
}

void *p2p_connection_handler(void *sock_desc)
{
	//work like server connection handler
	int sock = *(int*)sock_desc;
	char message[1024];
	recv(sock, message, 1024, 0);

	printf("\nYou have recieved a payment, please press (5) to receive\n");
	printf("press (5) again if you did not recieve payment\n\n");
	message[strlen(message)]= 0;
	memset(p2p_message, 0, sizeof(p2p_message));
	strcpy(p2p_message, message);
	memset(message, 0, sizeof(message));
	return 0;
}

void *sender_connection_handler(void *sock)
{
	int sock_desc;
	struct sockaddr_in peer;
	char thename[1024];
	char* buffer = (char *)sock;
	char* ip = strtok(buffer, "?");
	char* port = strtok(NULL, "?");
	char* msg = strtok(NULL, "?");
	
	sock_desc = socket(AF_INET , SOCK_STREAM , 0);
	if(sock_desc == -1)
	{
		printf("sender handler Could not create socket");
	}
		
	peer.sin_addr.s_addr = inet_addr(ip);
	peer.sin_family = AF_INET;
	peer.sin_port = htons(atoi(port));

	if(connect(sock_desc, (struct sockaddr *)&peer, sizeof(peer) ) < 0)
	{
		puts("sender handler connect error");
	}
	send(sock_desc, msg, strlen(msg), 0);
}
