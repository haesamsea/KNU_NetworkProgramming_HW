#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define INIT_VALUE 0
#define S_VALUE 1
#define C_VALUE 2
#define	BOARD_SIZE	3

typedef	struct	{
			int	board[BOARD_SIZE][BOARD_SIZE];
}GAMEBOARD;


#define BUF_SIZE 30
void error_handling(char *message);
void draw_board(GAMEBOARD *gboard);
int availble_space(GAMEBOARD *gboard);



/*
    서버 & 클라이언트 구현해야 할 것들
    1) 선택 가능한 공간이 있는지 확인하는 함수
    2) 선택 가능한 공간 중 랜덤으로 선택
    3) 입력된 값이 배열의 범위를 벗어나거나 빈공간이 아닌 경우,
    에러 처리 후 입력을 다시 받아야 함
*/

/*
클라이언트 구현
클라이언트는 해당 2차원 배열의 인덱스를 받고 오류 검사
클라이언트가 먼저 위치를 선택하고 데이터 전송
*/

int main(int argc, char *argv[])
{
	int sock;
	char message[BUF_SIZE];
	int str_len;
	socklen_t adr_sz;

    int send_row = 0;
    int send_col=0;
	int recv_row;
    int recv_col;
    GAMEBOARD play;

	struct sockaddr_in serv_adr, from_adr;
	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
    //udp socket 생성
	sock=socket(PF_INET, SOCK_DGRAM, 0);   
	if(sock==-1)
		error_handling("socket() error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));
	
    
    printf("Tic-Tac-Toe Client\n");
	draw_board(play.board);

    while(availble_space(play.board))
	{
        
        printf("Input row, column: ");
        scanf("%d %d", &send_row, &send_col);

        //if(!strcmp(send_row,"q\n") || !strcmp(send_row,"Q\n"))	
        //    break;     
        
        // 배열의 범위를 벗어남
        if ((0>send_row || send_row > 2) ||(send_col<0||send_col > 2)){
            printf("Wrong index, Input again!\n");
            continue;
        } 

        //빈 공간이 아님
        if ( play.board[send_row][send_col] == 1 || play.board[send_row][send_col] == 2){
            printf("Wrong index, Input again!\n");
            continue;
        }

        
        //문자열이나 바이트 배열로 변경 후 전송
        sendto(sock, &send_row, sizeof(send_row), 0, (struct sockaddr*)&serv_adr, sizeof(serv_adr));
        sendto(sock, &send_col, sizeof(send_col), 0, (struct sockaddr*)&serv_adr, sizeof(serv_adr));
        //adr_sz=sizeof(from_adr);

        play.board[send_row][send_col]=C_VALUE;
        draw_board(play.board);
        if (availble_space(play.board) == 0) break;

        int recv_byte;
        adr_sz = sizeof(from_adr);
        recv_byte = recvfrom(sock, &recv_row, sizeof(recv_row), 0, (struct sockaddr *)&from_adr, &adr_sz);
        if (recv_byte == -1) {
            perror("recvto(row) error - send_col");
        }
        recv_byte = recvfrom(sock, &recv_col, sizeof(recv_col), 0, (struct sockaddr *)&from_adr, &adr_sz);
        if (recv_byte == -1) {
            perror("recvto(row) error - send_col");
        }
    
        play.board[recv_row][recv_col]=S_VALUE;
        draw_board(play.board);

    }
	printf("No available space. Exit Client\n");
    printf("Tic Tac Toe Client Close\n");
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

int availble_space(GAMEBOARD *gboard){

    int flag = 0;
   for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (gboard->board[i][j] == S_VALUE)
                flag ++;
            else if (gboard->board[i][j] == C_VALUE)
                flag ++;
        }
    
    }

    if (flag > 8) {
        return 0; //빈공간이 없는 경우
    }
    else{
        return 1; // 빈공간이 있는 경우
    }
}

void draw_board(GAMEBOARD *gboard)
{
    char value = ' ';
    int i, j;

    printf("+-----------+\n");
    for (i = 0; i < BOARD_SIZE; i++)
    {
        for (j = 0; j < BOARD_SIZE; j++)
        {
            if (gboard->board[i][j] == INIT_VALUE)       // 초기값 0
                value = ' ';
            else if (gboard->board[i][j] == S_VALUE)     // Server 표시 (1)
                value = 'O';
            else if (gboard->board[i][j] == C_VALUE)     // Client 표시 (2)
                value = 'X';
            else
                value = ' ';
            
            printf("| %c ", value);
        }
        printf("|\n");
        printf("+-----------+\n");
    }
}
