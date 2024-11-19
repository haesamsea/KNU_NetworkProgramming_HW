//2022110257 오혜경
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>

int find(const char *dir_name, const char *file_name, char *found_path);
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

void get_current_datetime(char *buf)
{
    time_t t;
    time(&t);

    struct tm *p;
    p = localtime(&t);
    sprintf(buf, "%d-%d-%d %d:%d:%d",
            1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday,
            p->tm_hour, p->tm_min, p->tm_sec);
}

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    char message[BUF_SIZE];
    int str_len, i;
    int total_count = 0;
    int total_buf_count = 0;

    struct sockaddr_in serv_adr;
    struct sockaddr_in clnt_adr;
    socklen_t clnt_adr_sz;

    PACKET send_packet;
    PACKET recv_packet;

    memset(&send_packet, 0, sizeof(PACKET));
    memset(&recv_packet, 0, sizeof(PACKET));

    if (argc != 2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");

    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    clnt_adr_sz = sizeof(clnt_adr);

    clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
    /*if (clnt_sock == -1)
        error_handling("accept() error");
    else
       printf("Connected client_sock: %d \n", clnt_sock);
    */   
    //while ((str_len = read(clnt_sock, (void*)&recv_packet, sizeof(PACKET))) != 0)
	while(1)
    {
		str_len = read(clnt_sock, (void*)&recv_packet, sizeof(PACKET));
		if(str_len == 0) {
			break;
		}
        
        printf("---------------------------\n");
        printf("TCP Remote File View Server\n");
        printf("---------------------------\n");
    // 입력받은 파일 이름에서 맨 뒤 공백 제거
    recv_packet.buf[strcspn(recv_packet.buf, "\n")] = '\0';

#ifdef DEBUG
		printf("str_len: %d, cmd: %d\n", str_len, recv_packet.cmd);
#endif
		// recv_pakcet의 값이 초기화되어 cmd=0이 저장됨 
        if(recv_packet.cmd == FILE_REQ) 
        {
            char found_path[1024] = {0};
            printf("[RX] cmd: %d, file name: %s \n", recv_packet.cmd, recv_packet.buf);

            //find 함수로 탐색 후 존재할 때
            if(find(".", recv_packet.buf, found_path)){
                //printf("Found file %s\n", found_path);
                
                //---------파일 읽는 부분----------------------- 
                FILE *file = fopen(found_path, "r");
                if (file == NULL) {
                    perror("fopen");
                    break;
                }

                char buffer[BUF_SIZE+1];
                size_t bytes_read;

                // 파일의 내용을 읽고 출력
                while ((bytes_read = fread(buffer, 1, sizeof(buffer) - 1, file)) > 0) {
                    buffer[bytes_read] = '\0';  // null 문자 추가
                    strcpy(send_packet.buf, buffer);
                    send_packet.buf_len = strlen(buffer);
                    total_count++;
                    send_packet.cmd = FILE_RES;
                    total_buf_count += send_packet.buf_len;
                    
                    if(send_packet.buf_len < 100 && send_packet.buf_len > 0){
                        send_packet.cmd = FILE_END;
                        write(clnt_sock, (const void*)&send_packet, sizeof(PACKET));
                        break; // FILE_END 패킷을 보낸 후 종료
                    } 
                    printf("[Tx] cmd: %d, len: %3d, total_tx_cnt: %3d, total_tx_bytes: %d\n", send_packet.cmd, send_packet.buf_len, total_count, total_buf_count);
                    write(clnt_sock, (const void*)&send_packet, sizeof(PACKET)); 
                    memset(&send_packet.buf, 0, sizeof(BUF_SIZE));
                    
                }
                fclose(file);
                //----------파일 읽는 부분 끝----------------------
                //snprintf(send_packet.buf, sizeof(send_packet.buf), "Found: %s", found_path);
                memset(&send_packet, 0, sizeof(PACKET));
                
            }
            else {
                // 파일을 찾지 못한 경우
                send_packet.cmd = FILE_NOT_FOUND;
                //snprintf(send_packet.buf, sizeof(send_packet.buf), "%s: File Not Found", recv_packet.buf);
                printf("[TX] cmd: %d, %s: File Not Found\n", send_packet.cmd, recv_packet.buf);

                // 클라이언트에게 파일을 찾지 못했다는 패킷을 전송
                write(clnt_sock, (const void*)&send_packet, sizeof(PACKET));

                // 프로그램 종료 처리
                break;
            }
            read(clnt_sock, (void*)&recv_packet, sizeof(PACKET));

            if(recv_packet.cmd == FILE_END_ACK){
                printf("[Rx] cmd: %d, FILE_END_ACK\n", recv_packet.cmd);
                break;
            }
            else {
                send_packet.cmd = FILE_NOT_FOUND;
                printf("[TX] cmd: %d, %s: File Not Found\n" ,send_packet.cmd, recv_packet.buf);
                snprintf(send_packet.buf, sizeof(send_packet.buf), recv_packet.buf);
            }
        
            //write(clnt_sock, (void*)&send_packet, sizeof(PACKET));
        }else 
        {
            printf("Received a wrong message(cmd: %d)\n", recv_packet.cmd);
        }

		memset(&recv_packet, 0, sizeof(PACKET));
    }
    printf("---------------------------\n");
	printf("Total Tx count: %d, bytes: %d\n", total_count,total_buf_count);
    printf("TCP Server Socket Close!\n");
    printf("---------------------------\n");
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

int find(const char *dir_name, const char *file_name, char *found_path){ //디렉토리 경로, 파일 이름, 파일 발견 시 파일 경로 저장
    struct dirent *entry;
    DIR *dir = opendir(dir_name);

    //printf("we need to find %s\n", file_name);
    // 디렉토리 열기 실패
    if(dir == NULL){
        perror("opendir");
        return 0;
    }

    //디렉토리 탐색 (readdir)    
    while ((entry = readdir(dir))!=NULL){
        if(strcmp(entry->d_name, ".") == 0||strcmp(entry -> d_name,"..")==0){
            continue;
        }

        /*
        경로 생성 및 파일 정보 확인: stat

        */
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir_name, entry->d_name);
        
        //경로 생성 및 파일 정보 확인
        struct stat file_stat;
        if(stat(path, &file_stat) == -1){
            perror("stat"); //해당 경로의 파일 정보 얻기
            continue;
        }
        //확인용
        
        //printf("now file %s %d need to find %s %d \n", entry->d_name, strlen(entry->d_name), file_name, strlen(file_name));
        
        //디렉토리인지 파일인지 확인
        if (S_ISDIR(file_stat.st_mode)){
            if(find(path, file_name, found_path)){
                closedir(dir);
                return 1;
            }
        //파일을 찾은 경우
        }
        if (strcmp(entry->d_name, file_name)==0){
            strncpy(found_path,path,1024);
            closedir(dir);
            return 1;
        }
    }
    closedir(dir);
    return 0;
}

/*
10/4 구현 내용: file 이름으로 파일 찾기
10/5 구현해야 할 내용: file 읽어서 client한테 buf 전송
*/

