#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void error_handling(char *message);

#define BUF_SIZE 100

#define FILE_REQ 1
#define FILE_RES 2
#define FILE_END 3
#define FILE_END_ACK 4
#define FILE_NOT_FOUND 5

typedef struct 
{
    int cmd;
    int buf_len;
    char buf[BUF_SIZE];
} PACKET;

int main(int argc, char *argv[])
{
    int sock;
    char message[BUF_SIZE];
    int str_len;
    struct sockaddr_in serv_adr;
    PACKET send_packet;
    PACKET recv_packet;
    int total_count = 0;
    int total_buf_count = 0;
    char file_name[BUF_SIZE];

    memset(&send_packet, 0, sizeof(PACKET));
    memset(&recv_packet, 0, sizeof(PACKET));
    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("connect() error!");


    while (1)
    {
        fputs("Input file name: ", stdout);
        fgets(message, BUF_SIZE, stdin); // 파일 이름 입력

        if (!strcmp(message, "q\n") || !strcmp(message, "Q\n")) {
            break;
		}
        
        send_packet.cmd = FILE_REQ;
        strcpy(send_packet.buf, message);

        printf("[Tx] cmd: %d, file name: %s", send_packet.cmd, send_packet.buf);
        write(sock, (void *)&send_packet, sizeof(PACKET));            
		memset(&send_packet, 0, sizeof(PACKET));  
        
        // 서버로부터 패킷 읽기
        read(sock, (void*)&recv_packet, sizeof(PACKET));

        if(recv_packet.cmd == FILE_NOT_FOUND)
        {
            printf("[Rx] cmd: %d, %s: File Not Found\n", recv_packet.cmd, file_name);
            break;    
        }

        while(recv_packet.cmd == FILE_RES|| recv_packet.cmd == FILE_END) 
        {
            // 서버가 보낸 실제 바이트 수만큼만 출력
            fwrite(recv_packet.buf, 1, recv_packet.buf_len, stdout);

            total_count++;
            total_buf_count += recv_packet.buf_len;
            
            if(recv_packet.cmd == FILE_END) {
                printf("\n");
                send_packet.cmd = FILE_END_ACK;
                //read(sock, (void*)&recv_packet, sizeof(PACKET));
                printf("------------------\n");
                printf("[Rx] cmd: %d, FILE_END\n",recv_packet.cmd);
                printf("[Tx] cmd: %d, FILE_END_ACK\n",send_packet.cmd);
                write(sock, (void*)&send_packet, sizeof(PACKET));
                
                break;
            }
        memset(&send_packet, 0, sizeof(PACKET));  
        // 다음 패킷 읽기
        read(sock, (void*)&recv_packet, sizeof(PACKET));
        }
        break;

    }
    printf("----------------------\n");
    printf("Total Rx count: %d, bytes: %d\n", total_count,total_buf_count);
    printf("Tcp Client Socket Close!\n");
    printf("----------------------\n");
    close(sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}