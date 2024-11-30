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
    
    // 클라이언트 메시지 큐 생성
    if ((client_qid = msgget(CLIENT_KEY, IPC_CREAT | 0666)) == -1) {
        perror("클라이언트 메시지 큐 생성 실패");
        exit(1);
    }
    
    // 서버 메시지 큐 열기
    if ((server_qid = msgget(SERVER_KEY, 0666)) == -1) {
        perror("서버 메시지 큐 열기 실패");
        exit(1);
    }
    
    printf("채팅 클라이언트가 시작되었습니다.\n");
    printf("종료하려면 'quit'를 입력하세요.\n");
    
    while (1) {
        // 클라이언트 메시지 입력
        printf("클라이언트: ");
        fgets(msg.msg_text, MAX_TEXT, stdin);
        msg.msg_text[strlen(msg.msg_text) - 1] = '\0';  // 개행문자 제거
        msg.msg_type = 1;
        
        // 서버로 메시지 전송
        if (msgsnd(server_qid, &msg, strlen(msg.msg_text) + 1, 0) == -1) {
            perror("메시지 전송 실패");
            exit(1);
        }
        
        // 종료 메시지 확인
        if (strncmp(msg.msg_text, "quit", 4) == 0) {
            break;
        }
        
        // 서버로부터 응답 수신
        if (msgrcv(client_qid, &msg, sizeof(msg.msg_text), 0, 0) == -1) {
            perror("메시지 수신 실패");
            exit(1);
        }
        
        printf("서버: %s\n", msg.msg_text);
    }
    
    // 메시지 큐 삭제
    if (msgctl(client_qid, IPC_RMID, NULL) == -1) {
        perror("메시지 큐 삭제 실패");
        exit(1);
    }
    
    printf("채팅 클라이언트를 종료합니다.\n");
    return 0;
} 