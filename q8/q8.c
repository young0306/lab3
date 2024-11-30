#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

// 전역 변수로 카운터 선언
volatile int alarm_count = 0;
volatile int ctrl_c_count = 0;

// SIGALRM 핸들러
void alarm_handler(int signo) {
    alarm_count++;
    printf("\n[SIGALRM] 알람 시그널 발생! (%d번째)\n", alarm_count);
    
    if (alarm_count < 3) {
        // 다음 알람 설정 (2초)
        alarm(2);
    }
}

// SIGINT 핸들러 (Ctrl+C)
void sigint_handler(int signo) {
    ctrl_c_count++;
    printf("\n[SIGINT] Ctrl+C 감지! (%d번째)\n", ctrl_c_count);
    
    if (ctrl_c_count >= 3) {
        printf("Ctrl+C가 3번 감지되어 프로그램을 종료합니다.\n");
        exit(0);
    }
    printf("프로그램을 종료하려면 Ctrl+C를 %d번 더 입력하세요.\n", 3 - ctrl_c_count);
}

// SIGCHLD 핸들러
void sigchld_handler(int signo) {
    int status;
    pid_t pid;
    
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            printf("\n[SIGCHLD] 자식 프로세스 %d가 종료됨 (상태: %d)\n", 
                   pid, WEXITSTATUS(status));
        }
    }
}

// SIGUSR1 핸들러
void sigusr1_handler(int signo) {
    printf("\n[SIGUSR1] 사용자 정의 시그널 1 수신!\n");
}

int main() {
    pid_t pid;

    // 시그널 핸들러 등록
    signal(SIGALRM, alarm_handler);
    signal(SIGINT, sigint_handler);
    signal(SIGCHLD, sigchld_handler);
    signal(SIGUSR1, sigusr1_handler);

    printf("시그널 테스트 프로그램 시작 (PID: %d)\n", getpid());
    printf("다음 시그널들을 테스트합니다:\n");
    printf("1. SIGALRM (알람)\n");
    printf("2. SIGINT (Ctrl+C)\n");
    printf("3. SIGCHLD (자식 프로세스 종료)\n");
    printf("4. SIGUSR1 (사용자 정의)\n\n");

    // 첫 번째 알람 설정 (2초)
    alarm(2);

    // 자식 프로세스 생성
    pid = fork();

    if (pid < 0) {
        perror("fork 실패");
        exit(1);
    }
    else if (pid == 0) {  // 자식 프로세스
        printf("자식 프로세스 시작 (PID: %d)\n", getpid());
        sleep(5);  // 5초 동안 대기
        printf("자식 프로세스 종료\n");
        exit(0);
    }
    else {  // 부모 프로세스
        printf("부모 프로세스에서 자식 프로세스 (PID: %d)를 생성함\n", pid);
        
        // 3초 후에 자식 프로세스에게 SIGUSR1 시그널 전송
        sleep(3);
        printf("\n자식 프로세스에게 SIGUSR1 시그널 전송\n");
        kill(pid, SIGUSR1);

        // 메인 루프
        printf("\n아무 키나 입력하면 종료됩니다. (Ctrl+C 3번으로도 종료 가능)\n");
        while(1) {
            char c;
            if (read(STDIN_FILENO, &c, 1) > 0) {
                break;
            }
            sleep(1);
        }
    }

    printf("프로그램을 종료합니다.\n");
    return 0;
}

