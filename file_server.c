//2022110257 오혜경
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
#define SEQ_START 0

// cmd type
#define FILE_REQ 1			// server <-- client
#define FILE_RES 2			// server --> client 
#define FILE_END 3  		// server --> client
#define FILE_END_ACK 4		// server <-- client
#define FILE_NOT_FOUND 5 	// server --> client

typedef struct {
	int cmd;
	int seq;
	int ack;
	int buf_len;
	char buf[BUF_SIZE];

}PACKET;

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int serv_sock;
	int clnt_sock;
	int len;

	char fname[BUF_SIZE];
	int str_len;
	PACKET recv_packet, send_packet;

	socklen_t clnt_addr_size;
	struct sockaddr_in serv_addr, clnt_addr;

	FILE *fp = NULL;
	int fread_cnt = 0;
	int total_tx_cnt=0, total_tx_bytes=0;

	printf("------------------------------\n");
	printf("TCP Remote File View Server   \n");
	printf("------------------------------\n");

	if(argc!=2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(serv_sock==-1)
		error_handling("TCP socket creation error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("bind() error");

	if(listen(serv_sock, 5) == -1)
		error_handling("listen() error");
	
	clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
	if(clnt_sock == -1)
		error_handling("accept() error");

	// PACKET memory initialize 
	memset(&recv_packet, 0, sizeof(PACKET));
	memset(&send_packet, 0, sizeof(PACKET));
	memset(fname, 0, sizeof(char)*BUF_SIZE);
	
	while(1) 
	{
		read(clnt_sock, (void*)&recv_packet, sizeof(PACKET));
		/*--------------------------------------------------------
		          FILE_REQ	
		  Server  <------   Client 
		*---------------------------------------------------------*/
		if(recv_packet.seq == SEQ_START) {
			strncpy(fname, recv_packet.buf, recv_packet.buf_len);
			fname[recv_packet.buf_len] = 0; // NULL 추가 
			
			fp = fopen(fname, "rb");
			if(fp != NULL)
			{
				printf("[Server] sending %s\n\n", fname);

				while(!feof(fp))
				{
					fread_cnt = fread((void*)send_packet.buf, 1, BUF_SIZE, fp);
					
					total_tx_bytes += fread_cnt;
					total_tx_cnt++;
					// Not end of a file
					if(fread_cnt < BUF_SIZE)
					{
						send_packet.cmd = FILE_END;
						send_packet.buf_len = fread_cnt;
#ifdef DEBUG					
						printf("fread_cnt: %d\n", fread_cnt);
						//printf("%s", send_packet.buf);
#endif

						printf("[Server] Tx SEQ: %d, %d byte data\n", send_packet.seq , send_packet.buf_len);
						write(clnt_sock, (void*)&send_packet, sizeof(PACKET));
						printf("%s sent(%d Bytes)\n", fname, total_tx_bytes);
						break;
					}
					else
					{
						send_packet.cmd = FILE_RES;
						send_packet.buf_len = fread_cnt;

						printf("[Server] Tx SEQ: %d, %d byte data\n", send_packet.seq , send_packet.buf_len);
						
						write(clnt_sock, (void*)&send_packet, sizeof(PACKET));
						
					}
					read(clnt_sock,(void*)&recv_packet, sizeof(PACKET));
					printf("[Server] Rx ACK: %d\n", recv_packet.ack);
					memset(&send_packet, 0, sizeof(PACKET));
					send_packet.seq = recv_packet.ack;
					printf("\n");
					
				}
				break;
			}
			else 
			{
				send_packet.cmd = FILE_NOT_FOUND;
				printf("%s File Not Found\n", fname);
				break;
			}
		}
		
		memset(&recv_packet, 0, sizeof(PACKET));
	}	
	
	printf("Server Socket Close!\n");
	
	close(clnt_sock);
	close(serv_sock);

	return 0;
}
			
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
