#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#define MAX_TEXT 512
#define SERVER_KEY 1234
#define CLIENT_KEY 1235

// 메시지 구조체 정의
struct message {
    long msg_type;
    char msg_text[MAX_TEXT];
};

int main() {
    int server_qid, client_qid;
    struct message msg;
    
    // 서버 메시지 큐 생성
    if ((server_qid = msgget(SERVER_KEY, IPC_CREAT | 0666)) == -1) {
        perror("서버 메시지 큐 생성 실패");
        exit(1);
    }
    
    printf("채팅 서버가 시작되었습니다.\n");
    
    while (1) {
        // 클라이언트로부터 메시지 수신
        if (msgrcv(server_qid, &msg, sizeof(msg.msg_text), 0, 0) == -1) {
            perror("메시지 수신 실패");
            exit(1);
        }
        
        // 종료 메시지 확인
        if (strncmp(msg.msg_text, "quit", 4) == 0) {
            break;
        }
        
        printf("클라이언트: %s\n", msg.msg_text);
        
        // 서버 응답 입력
        printf("서버: ");
        fgets(msg.msg_text, MAX_TEXT, stdin);
        msg.msg_text[strlen(msg.msg_text) - 1] = '\0';  // 개행문자 제거
        msg.msg_type = 1;
        
        // 클라이언트 메시지 큐 열기
        if ((client_qid = msgget(CLIENT_KEY, 0666)) == -1) {
            perror("클라이언트 메시지 큐 열기 실패");
            exit(1);
        }
        
        // 클라이언트에게 메시지 전송
        if (msgsnd(client_qid, &msg, strlen(msg.msg_text) + 1, 0) == -1) {
            perror("메시지 전송 실패");
            exit(1);
        }
    }
    
    // 메시지 큐 삭제
    if (msgctl(server_qid, IPC_RMID, NULL) == -1) {
        perror("메시지 큐 삭제 실패");
        exit(1);
    }
    
    printf("채팅 서버를 종료합니다.\n");
    return 0;
} 