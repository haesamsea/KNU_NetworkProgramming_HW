//sender

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
//랜덤값 생성 샘플 헤더
#include <time.h>

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

int main(int argc, char *argv[])
{
	int serv_sock;
	char message[BUF_SIZE];
	int str_len;
    int total_len;
	socklen_t clnt_adr_sz;

    GAMEBOARD play;

    int send_row;
    int send_col;
    int recv_row;
    int recv_col;
    //서버가 선택한 위치의 값:1 클라이언트가 선택한 위치의 값:2
    //서버가 선택한 위치는 O, 클라이언트가 선택한 위치는 X
	
    /*
    서버 & 클라이언트 구현해야 할 것들
    1) 선택 가능한 공간이 있는지 확인하는 함수
    2) 선택 가능한 공간 중 랜덤으로 선택
    3) 입력된 값이 배열의 범위를 벗어나거나 빈공간이 아닌 경우,
    에러 처리 후 입력을 다시 받아야 함
    */

    /*
    서버 구현
    빈공간의 위치를 랜덤하게 설정
    빈공간이 나올 때까지 랜덤 함수를 호출하고 배열에 값을 저장한 다음
    클라이언트로 전송함(1초 간격, sleep(1) 적용)
    */
	struct sockaddr_in serv_adr, clnt_adr;
	if(argc!=2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
	serv_sock=socket(PF_INET, SOCK_DGRAM, 0);
	if(serv_sock==-1)
		error_handling("UDP socket creation error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
    //recvform()을 먼저 호출하는 곳에서 bind()
	if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");

    printf("Tic-Tac-Toe Server\n");

	while(1)
	{
        draw_board(play.board);

		clnt_adr_sz=sizeof(clnt_adr);
       	recvfrom(serv_sock, &recv_row, sizeof(recv_row), 0, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
		recvfrom(serv_sock, &recv_col, sizeof(recv_col), 0, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
        
        play.board[recv_row][recv_col]=C_VALUE;
        draw_board(play.board);
        
        srand(time(NULL));

        if (availble_space(play.board) == 0) break;
        while(1){
            send_row = rand() % 3;
            send_col = rand() % 3;
            
            if ( play.board[send_row][send_col] == C_VALUE || play.board[send_row][send_col] == S_VALUE)
                continue;
            break;
        }
        clnt_adr_sz = sizeof(clnt_adr);  // 구조체 크기 다시 확인
        
        sleep(1);
        printf("Server choose: [%d, %d]\n", send_row, send_col);	// Add 
		play.board[send_row][send_col]=S_VALUE;

        int send_bytes = sendto(serv_sock, &send_row, sizeof(send_row), 0, (struct sockaddr*)&clnt_adr, sizeof(clnt_adr));
        if (send_bytes == -1) {
            perror("sendto(row) error - send_col");
        }
        send_bytes=sendto(serv_sock, &send_col, sizeof(send_col), 0, (struct sockaddr*)&clnt_adr, sizeof(clnt_adr));
        if (send_bytes == -1) {
            perror("sendto(col) error - send_col");
        }

    }	
    printf("No available space. Exit this program.\n");
    printf("Tic Tac Toe Server Close\n");
	close(serv_sock);
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
