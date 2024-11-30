#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

void signal_handler(int signo) {
    if (signo == SIGCHLD) {
        printf("자식 프로세스가 종료되었습니다. (SIGCHLD 수신)\n");
    }
}

int main() {
    pid_t pid1, pid2;
    int status;

    signal(SIGCHLD, signal_handler);

    printf("1. 부모 프로세스 시작 (PID: %d)\n", getpid());
    fflush(stdout);

    pid1 = fork();
    
    if (pid1 < 0) {
        perror("fork() 실패");
        exit(1);
    }
    else if (pid1 == 0) {
        // 자식 프로세스 1
        printf("2. 자식 프로세스 1 시작 (PID: %d, PPID: %d)\n", 
               getpid(), getppid());
        printf("3. 자식 프로세스 1에서 ls 명령어 실행:\n");
        fflush(stdout);
        
        // 잠시 대기하여 출력 순서 조정
        usleep(100000);  // 0.1초 대기
        
        execl("/bin/ls", "ls", "-l", NULL);
        perror("execl() 실패");
        exit(1);
    }
    else {
        // 부모 프로세스에서 잠시 대기
        usleep(50000);  // 0.05초 대기
        
        printf("\n8. 부모 프로세스: 자식 프로세스들의 상태 관리\n");
        printf("9. 프로세스 그룹 ID: %d\n\n", getpgrp());
        fflush(stdout);

        pid2 = fork();
        
        if (pid2 < 0) {
            perror("fork() 실패");
            exit(1);
        }
        else if (pid2 == 0) {
            printf("4. 자식 프로세스 2 시작 (PID: %d, PPID: %d)\n", 
                   getpid(), getppid());
            
            int nice_val = nice(10);
            printf("5. 프로세스 우선순위 변경: %d\n", nice_val);
            printf("6. 자식 프로세스 2: 5초 동안 대기...\n");
            fflush(stdout);
            
            sleep(5);
            
            printf("7. 자식 프로세스 2 종료\n");
            fflush(stdout);
            exit(0);
        }
        else {
            // 첫 번째 자식 프로세스 대기
            waitpid(pid1, &status, 0);
            if (WIFEXITED(status)) {
                printf("10. 자식 프로세스 1이 정상 종료됨 (종료 코드: %d)\n", 
                       WEXITSTATUS(status));
                fflush(stdout);
            }
            
            // 두 번째 자식 프로세스 대기
            waitpid(pid2, &status, 0);
            if (WIFEXITED(status)) {
                printf("11. 자식 프로세스 2가 정상 종료됨 (종료 코드: %d)\n", 
                       WEXITSTATUS(status));
                fflush(stdout);
            }
            
            printf("\n12. 프로세스 정보:\n");
            printf("    - 현재 프로세스 ID (PID): %d\n", getpid());
            printf("    - 부모 프로세스 ID (PPID): %d\n\n", getppid());
            printf("13. 모든 프로세스 작업 완료\n");
            fflush(stdout);
        }
    }

    return 0;
} 
