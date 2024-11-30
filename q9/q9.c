#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_SIZE 100

int main() {
    int pipe_fd1[2];  // 부모 -> 자식 파이프
    int pipe_fd2[2];  // 자식 -> 부모 파이프
    pid_t pid;
    char buffer[BUFFER_SIZE];

    // 두 개의 파이프 생성
    if (pipe(pipe_fd1) == -1 || pipe(pipe_fd2) == -1) {
        perror("파이프 생성 실패");
        exit(1);
    }

    pid = fork();
    if (pid < 0) {
        perror("fork 실패");
        exit(1);
    }

    if (pid > 0) {  // 부모 프로세스
        // 사용하지 않는 파이프 끝 닫기
        close(pipe_fd1[0]);  // 첫 번째 파이프의 읽기 끝
        close(pipe_fd2[1]);  // 두 번째 파이프의 쓰기 끝

        printf("부모 프로세스 시작 (PID: %d)\n", getpid());

        // 자식에게 메시지 보내기
        const char *parent_msg = "안녕, 자식 프로세스!";
        printf("\n부모: 자식에게 메시지 전송 -> %s\n", parent_msg);
        write(pipe_fd1[1], parent_msg, strlen(parent_msg) + 1);

        // 자식으로부터 응답 받기
        read(pipe_fd2[0], buffer, BUFFER_SIZE);
        printf("부모: 자식으로부터 메시지 수신 -> %s\n", buffer);

        // 두 번째 메시지 전송
        const char *parent_msg2 = "잘 지내니?";
        printf("\n부모: 두 번째 메시지 전송 -> %s\n", parent_msg2);
        write(pipe_fd1[1], parent_msg2, strlen(parent_msg2) + 1);

        // 두 번째 응답 받기
        read(pipe_fd2[0], buffer, BUFFER_SIZE);
        printf("부모: 두 번째 응답 수신 -> %s\n", buffer);

        // 파이프 닫기
        close(pipe_fd1[1]);
        close(pipe_fd2[0]);

        // 자식 프로세스 종료 대기
        wait(NULL);
        printf("\n부모 프로세스 종료\n");
    }
    else {  // 자식 프로세스
        // 사용하지 않는 파이프 끝 닫기
        close(pipe_fd1[1]);  // 첫 번째 파이프의 쓰기 끝
        close(pipe_fd2[0]);  // 두 번째 파이프의 읽기 끝

        printf("자식 프로세스 시작 (PID: %d)\n", getpid());

        // 부모로부터 메시지 받기
        read(pipe_fd1[0], buffer, BUFFER_SIZE);
        printf("자식: 부모로부터 메시지 수신 -> %s\n", buffer);

        // 부모에게 응답 보내기
        const char *child_msg = "안녕하세요, 부모 프로세스!";
        printf("자식: 응답 메시지 전송 -> %s\n", child_msg);
        write(pipe_fd2[1], child_msg, strlen(child_msg) + 1);

        // 두 번째 메시지 받기
        read(pipe_fd1[0], buffer, BUFFER_SIZE);
        printf("자식: 두 번째 메시지 수신 -> %s\n", buffer);

        // 두 번째 응답 보내기
        const char *child_msg2 = "네, 잘 지내요!";
        printf("자식: 두 번째 응답 전송 -> %s\n", child_msg2);
        write(pipe_fd2[1], child_msg2, strlen(child_msg2) + 1);

        // 파이프 닫기
        close(pipe_fd1[0]);
        close(pipe_fd2[1]);

        printf("자식 프로세스 종료\n");
        exit(0);
    }

    return 0;
} 
