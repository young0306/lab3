#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#define MAX_INPUT 1024
#define MAX_ARGS 64

// 백그라운드 프로세스 정보를 저장하는 구조체
struct bg_process {
    pid_t pid;
    char command[MAX_INPUT];
};

// 백그라운드 프로세스 목록
struct bg_process bg_processes[100];
int bg_count = 0;

// SIGCHLD 핸들러: 종료된 백그라운드 프로세스 처리
void sigchld_handler(int signo) {
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < bg_count; i++) {
            if (bg_processes[i].pid == pid) {
                printf("\n[%d] 완료    %s\n", pid, bg_processes[i].command);

                // 완료된 프로세스 제거
                for (int j = i; j < bg_count - 1; j++) {
                    bg_processes[j] = bg_processes[j + 1];
                }
                bg_count--;
                break;
            }
        }
    }
}

// SIGINT 핸들러: Ctrl-C 처리
void sigint_handler(int signo) {
    printf("\nSIGINT 수신: 프로세스 종료\n");
}

// SIGQUIT 핸들러: Ctrl-Z 처리
void sigquit_handler(int signo) {
    printf("\nSIGQUIT 수신: 프로세스 일시 중지\n");
}

// 문자열을 공백 기준으로 분리
int parse_input(char *input, char **args) {
    int i = 0;
    int background = 0;

    // 문자열을 공백으로 분리
    char *token = strtok(input, " \n");
    while (token != NULL && i < MAX_ARGS - 1) {
        if (strcmp(token, "&") == 0) {
            background = 1;  // 백그라운드 실행 플래그 설정
            break;
        }
        args[i++] = token;
        token = strtok(NULL, " \n");
    }
    args[i] = NULL;  // 명령어 배열의 끝 설정

    // 입력 문자열의 끝에 '&'가 포함된 경우 처리
    if (i > 0 && args[i - 1][strlen(args[i - 1]) - 1] == '&') {
        args[i - 1][strlen(args[i - 1]) - 1] = '\0';  // '&' 제거
        background = 1;
    }

    return background;
}


// 명령어가 파일 재지향인지 확인
void handle_redirection(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            // 출력 재지향
            int fd = open(args[i + 1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
            if (fd == -1) {
                perror("출력 파일 열기 실패");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;
        } else if (strcmp(args[i], "<") == 0) {
            // 입력 재지향
            int fd = open(args[i + 1], O_RDONLY);
            if (fd == -1) {
                perror("입력 파일 열기 실패");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
            args[i] = NULL;
        }
    }
}

// 파이프 명령어 처리
void handle_pipe(char *input) {
    char *commands[MAX_ARGS];
    int num_commands = 0;
    char *token = strtok(input, "|");

    // 파이프 분리
    while (token != NULL && num_commands < MAX_ARGS) {
        commands[num_commands++] = token;
        token = strtok(NULL, "|");
    }

    int pipefds[2 * (num_commands - 1)];
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipefds + i * 2) == -1) {
            perror("파이프 생성 실패");
            exit(1);
        }
    }

    for (int i = 0; i < num_commands; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("프로세스 생성 실패");
            exit(1);
        } else if (pid == 0) {
            // 자식 프로세스
            if (i > 0) {
                dup2(pipefds[(i - 1) * 2], STDIN_FILENO); // 이전 파이프 읽기
            }
            if (i < num_commands - 1) {
                dup2(pipefds[i * 2 + 1], STDOUT_FILENO); // 다음 파이프로 쓰기
            }

            for (int j = 0; j < 2 * (num_commands - 1); j++) {
                close(pipefds[j]);
            }

            char *args[MAX_ARGS];
            parse_input(commands[i], args);
            handle_redirection(args);

            if (execvp(args[0], args) == -1) {
                perror("명령어 실행 실패");
                exit(1);
            }
        }
    }

    // 부모 프로세스: 파이프 닫기
    for (int i = 0; i < 2 * (num_commands - 1); i++) {
        close(pipefds[i]);
    }
    for (int i = 0; i < num_commands; i++) {
        wait(NULL);
    }
}

void print_prompt() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("\n%s $ ", cwd);
    } else {
        printf("\n$ ");
    }
    fflush(stdout);
}

// jobs 명령어 처리
void show_jobs() {
    if (bg_count == 0) {
        printf("실행 중인 백그라운드 프로세스가 없습니다.\n");
        return;
    }

    printf("\n실행 중인 백그라운드 프로세스 목록:\n");
    for (int i = 0; i < bg_count; i++) {
        printf("[%d] 실행 중  %s\n", bg_processes[i].pid, bg_processes[i].command);
    }
}

int main() {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];
    pid_t pid;

    // 시그널 핸들러 등록
    signal(SIGCHLD, sigchld_handler);
    signal(SIGINT, sigint_handler);  // Ctrl-C
    signal(SIGQUIT, sigquit_handler); // Ctrl-Z

    printf("쉘 프로그램을 시작합니다.\n");
    printf("'exit'를 입력하면 종료됩니다.\n");
    printf("명령어 뒤에 '&'를 붙이면 백그라운드로 실행됩니다.\n");
    printf("'jobs'로 백그라운드 프로세스 목록을 확인할 수 있습니다.\n");

    while (1) {
        print_prompt();
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break;
        }

        // 빈 입력 처리
        if (strlen(input) == 1) {
            continue;
        }

        // exit 명령어 처리
        input[strcspn(input, "\n")] = 0;
        if (strcmp(input, "exit") == 0) {
            printf("쉘을 종료합니다.\n");
            break;
        }

        // jobs 명령어 처리
        if (strcmp(input, "jobs") == 0) {
            show_jobs();
            continue;
        }

        // 파이프 처리
        if (strchr(input, '|') != NULL) {
            handle_pipe(input);
            continue;
        }

        // 입력을 파싱하고 백그라운드 실행 여부 확인
        int background = parse_input(input, args);

        pid = fork();
        if (pid < 0) {
            perror("fork 실패");
            continue;
        } else if (pid == 0) {
            // 자식 프로세스
            handle_redirection(args);

            if (execvp(args[0], args) == -1) {
                perror("명령어 실행 실패");
                exit(1);
            }
        } else {
            // 부모 프로세스
            if (background) {
                // 백그라운드 프로세스 정보 저장
                bg_processes[bg_count].pid = pid;
                strcpy(bg_processes[bg_count].command, input);
                printf("[%d] %d\n", bg_count + 1, pid);
                bg_count++;
            } else {
                // 포그라운드 실행: 자식 프로세스 종료 대기
                int status;
                waitpid(pid, &status, 0);
            }
        }
    }
    return 0;
}
