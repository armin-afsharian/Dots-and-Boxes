#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h>
#include <stdlib.h>  

#define MAXPORTSIZE 5
#define PORTNUMINIT 5

void initialize_map(int num_player, char row[num_player + 1][num_player + 1], char column[num_player + 1][num_player + 1], 
					char Char[num_player + 1][num_player + 1]);

void print_map(int num_player, char row[num_player + 1][num_player + 1], char column[num_player + 1][num_player + 1], 
					char Char[num_player + 1][num_player + 1]);

int update_game(int num_player, char row[num_player + 1][num_player + 1], char column[num_player + 1][num_player + 1], 
					char Char[num_player + 1][num_player + 1], int v_or_w, int x, int y, char symbol);

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

int main(int argc, char  *argv[]) 
{ 
	int sock = 0,msg_size, port;
	int opt = 1;
	struct sockaddr_in serv_addr;
	char buffer[1024] = {0}; 
	int broadcast = 1;
	port = extract_port(argv[1]);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(port); 
	inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
	connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    read( sock , buffer, 1024);
    printf("%s\n",buffer );
	scanf("%s", buffer);
	send(sock, buffer, strlen(buffer), 0);
	int num_players = atoi(buffer);
	msg_size = read( sock , buffer, 1024);
	buffer[msg_size] = '\0';
    printf("%s\n",buffer );
	memset(buffer, '\0', sizeof buffer);
	msg_size = read( sock , buffer, 1000);
	buffer[msg_size] = '\0';
	int udp_port = atoi(buffer);
	memset(buffer, '\0', sizeof buffer);
	msg_size = read( sock , buffer, 1024);
	buffer[msg_size] = '\0';
	int turn = atoi(buffer);
	printf("you are player number : %d\n", turn);
	char symbol = turn + '0';
	int game_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int broadcastPermission = 1;
	setsockopt(game_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  sizeof(opt));
	setsockopt(game_sock, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission, sizeof(broadcastPermission));
	struct sockaddr_in broadcastAddr;
	memset(&broadcastAddr, 0, sizeof(broadcastAddr));
	broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_addr.s_addr = inet_addr("255.255.255.255");
    broadcastAddr.sin_port = htons(udp_port);
	bind(game_sock, (struct sockaddr *) &broadcastAddr, sizeof(broadcastAddr));
	char row[num_players + 1][num_players + 1];
	char column[num_players + 1][num_players + 1];
	char Char[num_players + 1][num_players + 1];
	int scores[4] = {0 , 0, 0, 0};
	initialize_map(num_players, row, column, Char);
	if(turn == 1)
	{
		write(1, "start the game\n\n", strlen("start the game\n\n"));
		print_map(num_players, row, column, Char);
	}
	fd_set fds;
	while (1)
	{
		FD_ZERO(&fds);
		FD_SET(game_sock, &fds);
		FD_SET(0, &fds);
		select(game_sock + 1 , &fds , NULL , NULL , NULL);
		if(FD_ISSET(game_sock, &fds))
		{
			memset(buffer, '\0', sizeof buffer);
			msg_size = recvfrom(game_sock, buffer, 1024, 0, NULL, 0);
			buffer[msg_size] = '\0';
			int v_or_w = buffer[0] - '0';
			int x = buffer[2] - '0';
			int y = buffer[4]  - '0';
			char temp = buffer[6];
			int n = temp - '0';
			int point_won = update_game(num_players, row, column, Char, v_or_w, x, y, temp);
			if(point_won == 0)
			{
				if(turn == 1)
					turn = num_players;
				else
					turn--;
			}
			if(point_won == 1)
				scores[n - 1] += 1;
			if((scores[0] + scores[1] + scores[2] + scores[3]) == (num_players)*(num_players))
				break;
			if(turn == 1)
				write(1, "It is your turn.\n\n", strlen("It is your turn.\n\n"));
		}
		else if(FD_ISSET(0, &fds))
		{
			if(turn == 1)
			{
				memset(buffer, '\0', sizeof buffer);
				msg_size = read(0, buffer, 1024);
				buffer[msg_size] = symbol;
				buffer[msg_size + 1] = '\0';
				sendto(game_sock, buffer, strlen(buffer), 0, (struct sockaddr *) &broadcastAddr, sizeof(broadcastAddr));
			}
		}
	}
	int max = 0;
	int index;
	int draw = 0;
	for(int i = 0; i < num_players; i++)
	{
		if(scores[i] > max)
		{
			index = i + 1;
			max = scores[i];
			draw = 0;
		}
		else if(scores[i] == max)
			draw = 1;
		printf("player %d has %d scores\n", i + 1, scores[i]);
	}
	if(draw == 0)
		printf("player %d is the winner!!!\n", index);
	else
		printf("draw!!\n");
	return 0; 
}

void initialize_map(int num_player, char row[num_player + 1][num_player + 1], char column[num_player + 1][num_player + 1], 
					char Char[num_player + 1][num_player + 1])
{
	
	for (int i = 0; i < num_player + 1; i++) 
	{
		for (int j = 0; j < num_player + 1; j++) 
		{
			row[i][j] = ' ';
			column[i][j] = ' ';
			Char[i][j] = ' ';
		}
	}
}

void print_map(int num_player, char row[num_player + 1][num_player + 1], char column[num_player + 1][num_player + 1], 
					char Char[num_player + 1][num_player + 1])
{
	for (int i = 0; i < num_player + 1; i++) 
	{
		for (int j = 0; j < num_player + 1; j++)
		{
			printf("*%c%c", row[i][j], row[i][j]);
		}
		printf("\n");
		for (int j = 0; j < num_player + 1; j++)
		{
			printf("%c%c ", column[i][j], Char[i][j]);
		}
		printf("\n");
	}
}

int update_game(int num_player, char row[num_player + 1][num_player + 1], char column[num_player + 1][num_player + 1], 
					char Char[num_player + 1][num_player + 1], int v_or_w, int x, int y, char symbol)
{
	if (v_or_w == 1)
		column[x - 1][y - 1] = '|';
	else if (v_or_w == 0)
		row[x - 1][y - 1] = '-';

	int point_won = 0;
	if (v_or_w == 1)
	{
		if ((column[x - 1][y - 1] == '|') &&(column[x - 1][y - 2] == '|') && (row[x - 1][y - 2] == '-') && (row[x][y - 2] == '-'))
		{
			Char[x - 1][y - 2] = symbol;
			point_won = 1;
		}
		if ((column[x - 1][y - 1] == '|') && (column[x - 1][y] == '|') && (row[x - 1][y - 1] == '-') && (row[x][y - 1] == '-'))
		{
			Char[x - 1][y - 1] = symbol;
			point_won = 1;
		}
	}

	if (v_or_w == 0)
	{
		if ((row[x - 1][y - 1] == '-')&&(column[x - 2][y - 1] == '|') && (column[x - 2][y] == '|') && (row[x - 2][y - 1]))
		{
			Char[x - 2][y - 1] = symbol;
			point_won = 1;
		}
		if ((row[x - 1][y - 1] == '-') && (column[x - 1][y - 1] == '|') && (column[x - 1][y] == '|') && (row[x][y - 1] == '-'))
		{
			Char[x - 1][y - 1] = symbol;
			point_won = 1;
		}
	}

	write(1, "Map has been updated \n\n", strlen("Map has been updated \n\n"));
	print_map(num_player, row, column, Char);
	return point_won;
}