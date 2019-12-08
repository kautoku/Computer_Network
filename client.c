#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>	//function inet_addr : convert IP address
#include <unistd.h> //read

int main(int argc , char *argv[])
{
	int socket_desc;
	struct sockaddr_in server;
	char server_reply[1024];
	
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
	while(1)
	{
		char cmd[20], message[20]; // command only as q/1/2/3
		printf("You can:\n");
		printf("(q) Quit and do nothing.\n");
		printf("(1) Register for Micropayment System.\n");
		printf("(2) Log in to system.\n");
		printf("(3) See the latest online client list\n");
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
		else if (strcmp(cmd, "1") == 0)
		{
			char name[10], message[20];
			printf("Enter your register name(2 to 10 letters).\n");
			printf("New username: ");
			scanf("%s", name);
			//message: "REGISTER#name\n"
			strcpy(message, "REGISTER#");
			strcat(message, name);
			strcat(message, "\n");
			printf("\n");
		}
		else if (strcmp(cmd, "2") == 0)
		{
			char message[20], port[10];
			printf("Enter your username and port number.\n");
			printf("login username: ");
			scanf("%s", message);
			printf("port number: ");
			scanf("%s", port);
			//message: "name#1234\n"
			strcat(message, "#");
			strcat(message, port);
			strcat(message, "\n");
			printf("\n");
		}
		else if (strcmp(cmd, "3") == 0)
		{
			//message: "List\n"
			strcpy(message, "List\n");
		}
		else /* default: */
		{
			printf("command not found: %s Please re-enter.\n\n", cmd);
			continue;
		}
		send(socket_desc, message, strlen(message), 0);
		read(socket_desc, server_reply, 1024);
		puts(server_reply);
		memset(server_reply, 0, sizeof(server_reply));
	}
	close(socket_desc);
	return 0;
}
