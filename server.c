#include <stdio.h> 
#include <string.h>  
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h> 
#include <arpa/inet.h>  
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h>


#define OPTVALUE 1
#define MAXPORTSIZE 5
#define PORTNUMINIT 5
#define MAXPENDINGCON 4
#define HELLOMASSAGE "Hello, you are connencted to server , Enter number of players :"
#define SUCCESSFULADD "You added to game successfully! Please, Wait for game to start."
#define FULLGAME "Your game is full now, Enjoy!"
#define TRUE 1
#define FALSE 0
#define TWOPLAYER 2
#define THREEPLAYER 3
#define FOURPLAYER 4

//extract port from argv
int extract_port(char* string) 
{
	char port[MAXPORTSIZE];
	int result, i = PORTNUMINIT;
	while (string[i] != '\0')
	{
		port[i - MAXPORTSIZE] = string[i];
		i++;
	}
	result = atoi(port);
	return result;
}
void game_is_full(int game[], int n, int* port_clients);

void add_player_to_game(char* buffer, int sd, int two_player[], int three_player[], int four_player[], int* port_clients)
{
	if(strcmp(buffer, "2") == 0)
	{
		for(int i = 0; i < TWOPLAYER; i++)
		{
			if(two_player[i] == 0)
			{
				two_player[i] = sd;
				send(sd, SUCCESSFULADD, strlen(SUCCESSFULADD), 0);
				printf("Player added to game.\n");	
				if(i == TWOPLAYER - 1)
				{
					printf("%d player game is ready\n", TWOPLAYER);
					sleep(1);
					game_is_full(two_player, TWOPLAYER, port_clients);
				}
				break;
			}
		}
	}

	else if(strcmp(buffer, "3") == 0)
	{
		for(int i = 0; i < THREEPLAYER; i++)
		{
			if(three_player[i] == 0)
			{
				three_player[i] = sd;
				send(sd, SUCCESSFULADD, strlen(SUCCESSFULADD), 0);
				printf("Player added to game.\n");	
				if(i == THREEPLAYER - 1)
				{
					printf("%d player game is ready\n", THREEPLAYER);
					sleep(1);
					game_is_full(three_player, THREEPLAYER, port_clients);
				}
				break;
			}
		}
	}
	else if(strcmp(buffer, "4") == 0)
	{
		for(int i = 0; i < FOURPLAYER; i++)
		{
			if(four_player[i] == 0)
			{
				four_player[i] = sd;
				send(sd, SUCCESSFULADD, strlen(SUCCESSFULADD), 0);
				printf("Player added to game.\n");	
				if(i == FOURPLAYER - 1)
				{
					printf("%d player game is ready\n", FOURPLAYER);
					sleep(1);
					game_is_full(four_player, FOURPLAYER, port_clients);
				}
				break;
			}
		}
	}
}
void tostring(char str[], int num)
{
    int i, rem, len = 0, n;
 
    n = num;
    while (n != 0)
    {
        len++;
        n /= 10;
    }
    for (i = 0; i < len; i++)
    {
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem + '0';
    }
    str[10] = '\n';
}

void game_is_full(int game[], int n, int* port_clients)
{
	char string[11];
	tostring(string, *port_clients);
	for(int i = 0; i < n; i++)
	{
		int temp_sd = game[i];
		send(temp_sd, string, strlen(string), 0);
		sleep(1);
		char turn[11];
		tostring(turn, i + 1);
		send(temp_sd, turn, strlen(turn), 0);
		game[i] = 0;
	}
	(*port_clients)++;
}
int main(int argc , char *argv[]) 
{ 
	int server_socket, opt = OPTVALUE, port;
	struct sockaddr_in address;
	server_socket = socket(AF_INET, SOCK_STREAM, 0); //build socket
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  sizeof(opt)); //usable for multi-users
	port = extract_port(argv[1]); 
	printf("Server has connected to port %d \n", port);
	address.sin_family = AF_INET;   
    address.sin_addr.s_addr = INADDR_ANY;   
    address.sin_port = htons(port);
	bind(server_socket, (struct sockaddr *)&address, sizeof(address));
	listen(server_socket, MAXPENDINGCON); // make server ready for connection
	printf("Server is ready for connection \n");
	fd_set fds;
	int address_len = sizeof(address);
	int* client_sds;
	int clients_sds_size = 0, max_sds;
	client_sds = (int*)calloc(clients_sds_size, sizeof(int));
	char buffer[1024] = {0};
	int msg_size;
	int two_player[2] = {0 ,0};
	int three_player[3] = {0, 0, 0};
	int four_player[4] = {0, 0, 0, 0};
	int port_clients = 3000;
	while (1)
	{
		FD_ZERO(&fds);
		FD_SET(server_socket, &fds);
		max_sds = server_socket;
		for (int i = 0 ; i < clients_sds_size ; i++)   
        {    
            int sd = client_sds[i];   
            if(sd > 0)   
                FD_SET(sd, &fds);
            if(sd > max_sds)   
                max_sds = sd;
        }
		select(max_sds + 1 , &fds , NULL , NULL , NULL);
		if(FD_ISSET(server_socket, &fds))
		{
			int new_socket = accept(server_socket,  (struct sockaddr *)&address, (socklen_t*)&address_len);
			printf("New connection , socket fd is %d , ip is : %s , port : %d  \n", 
			new_socket , inet_ntoa(address.sin_addr) , ntohs (address.sin_port)); 
			send(new_socket, HELLOMASSAGE, strlen(HELLOMASSAGE), 0);
			int need_realloc = TRUE;
			for (int i = 0; i < clients_sds_size; i++)   
            {     
                if( client_sds[i] == 0 )   
                {   
                    client_sds[i] = new_socket;
					need_realloc = FALSE; 
					break;
                } 
            }
			if(need_realloc == TRUE)
			{
				clients_sds_size++;
				client_sds = realloc(client_sds, clients_sds_size * sizeof(int));
				client_sds[clients_sds_size - 1] = new_socket;
			}
		}
		for(int i = 0; i < clients_sds_size; i++)
		{
			int temp_sd = client_sds[i];
            if (FD_ISSET(temp_sd , &fds))   
            {   
				msg_size = read( temp_sd , buffer, 1024);
                if (msg_size == 0)
                {     
                    getpeername(temp_sd , (struct sockaddr*)&address , (socklen_t*)&address_len);   
                    printf("Client disconnected , ip %s , port %d \n" ,  
                          inet_ntoa(address.sin_addr) , ntohs(address.sin_port));   
                    close(temp_sd);   
                    client_sds[i] = 0;   
                }   
                     
                else 
                {   
                    buffer[msg_size] = '\0';   
                    printf("New message from client ip %s , port %d : %s \n",
					inet_ntoa(address.sin_addr), ntohs(address.sin_port), buffer);
					add_player_to_game(buffer, temp_sd, two_player, three_player, four_player, &port_clients);
					printf("%d \n", port_clients);
                }   
			}
		}
	}
	
	
	return 0;
} 
