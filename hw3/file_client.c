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
#define FILE_REQ 1
#define FILE_RES 2
#define FILE_END 3
#define FILE_END_ACK 4
#define FILE_NOT_FOUND 5 

typedef struct {
	int cmd; 
	int seq;
	int ack;
	int buf_len; // 실제 전송되는 파일 내용의 크기 저장 
	char buf[BUF_SIZE];
}PACKET;

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int sock;
	char fname[BUF_SIZE];
	int str_len;
	socklen_t addr_size;
	//PACKET recv_packet, send_packet;
	PACKET send_packet, recv_packet;	// 순서 변경 
	struct sockaddr_in serv_addr;
	int total_ack=0, total_rx_bytes=0;

	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock = socket(PF_INET, SOCK_STREAM, 0);   
	if(sock == -1)
		error_handling("socket() error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	
	// PACKET memory initialize
	memset(&recv_packet, 0, sizeof(PACKET));
	memset(&send_packet, 0, sizeof(PACKET));
	memset(fname, 0, sizeof(char) * BUF_SIZE);
	
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error!");

	printf("Input file name: ");
	scanf("%s", fname);
	
	/*-----------------------------------------------------------
	              FILE_REQ 
	      Client  ------->  Server
	           (buf: file name)
	*-----------------------------------------------------------*/
	send_packet.seq = SEQ_START;
	send_packet.buf_len = strlen(fname);
	strncpy(send_packet.buf, fname, send_packet.buf_len); 
	
	write(sock, (void*)&send_packet, sizeof(PACKET));

	printf("[Client] request: %s\n\n", send_packet.buf);	

	while(1) {

		read(sock, (void*)&recv_packet, sizeof(PACKET));

		if(recv_packet.cmd == FILE_RES)
		{
			total_rx_bytes += recv_packet.buf_len;

			printf("[Client] Rx Seq: %d, len: %d bytes\n", recv_packet.seq, recv_packet.buf_len);
			send_packet.ack = total_ack + recv_packet.buf_len + 1;
			write(sock, (void*)&send_packet, sizeof(PACKET));
			total_ack = send_packet.ack;
			printf("[Client] Tx ACK: %d\n", send_packet.ack);
		}
		else if(recv_packet.cmd == FILE_NOT_FOUND)
		{
			printf("File Not Found\n");
			break;
		}
		else if(recv_packet.cmd == FILE_END)
		{
			total_rx_bytes += recv_packet.buf_len;
			printf("[Client] Rx Seq: %d, len: %d bytes\n", recv_packet.seq, recv_packet.buf_len);
			
			send_packet.ack = total_ack + recv_packet.buf_len + 1;
			write(sock, (void*)&send_packet, sizeof(PACKET));
			total_ack = send_packet.ack;
			//printf("[Client] Tx ACK: %d\n", send_packet.ack);

			send_packet.cmd = FILE_END_ACK;
			write(sock, (void*)&send_packet, sizeof(PACKET));
			
			break;
		}
		else
		{
			printf("File Not Found\n");
			break;
		}

		memset(&recv_packet, 0, sizeof(PACKET));
		memset(&send_packet, 0, sizeof(PACKET));
		printf("\n");

	}
	if(total_rx_bytes>0)
		printf("%s received (%d Bytes)\n",fname, total_rx_bytes);
	printf("Client Socket Close!\n");
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
