#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_INPUT 1024
#define MAX_ARGS 64

// SIGCHLD 핸들러
void sigchld_handler(int signo) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

// 입력 파싱
int parse_input(char *input, char **args) {
    int i = 0;
    char *token = strtok(input, " \n");
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \n");
    }
    args[i] = NULL;
    return i;
}

// 프롬프트 출력
void print_prompt() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("\n%s $ ", cwd);
    } else {
        printf("\n$ ");
    }
    fflush(stdout);
}

// 리다이렉션 처리
void redirect_io(char *input_file, char *output_file) {
    if (input_file) {
        int fd = open(input_file, O_RDONLY);
        if (fd < 0) {
            perror("입력 파일 열기 실패");
            exit(1);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    if (output_file) {
        int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            perror("출력 파일 열기 실패");
            exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}

// 파이프 및 리다이렉션 처리
void execute_piped_commands(char *input) {
    char *commands[MAX_ARGS];
    int num_commands = 0;

    // 파이프 기준으로 명령어 분리
    char *command = strtok(input, "|");
    while (command != NULL && num_commands < MAX_ARGS - 1) {
        commands[num_commands++] = command;
        command = strtok(NULL, "|");
    }
    commands[num_commands] = NULL;

    int pipe_fd[2];
    int prev_fd = -1;

    for (int i = 0; i < num_commands; i++) {
        if (pipe(pipe_fd) == -1) {
            perror("파이프 생성 실패");
            exit(1);
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork 실패");
            exit(1);
        }

        if (pid == 0) {  // 자식 프로세스
            // 리다이렉션 파일 설정
            char *input_file = NULL, *output_file = NULL;
            char *args[MAX_ARGS];
            parse_input(commands[i], args);

            for (int j = 0; args[j] != NULL; j++) {
                if (strcmp(args[j], "<") == 0) {
                    input_file = args[j + 1];
                    args[j] = NULL;
                } else if (strcmp(args[j], ">") == 0) {
                    output_file = args[j + 1];
                    args[j] = NULL;
                }
            }

            if (prev_fd != -1) {
                dup2(prev_fd, STDIN_FILENO);  // 이전 파이프의 읽기 끝을 표준 입력으로 리다이렉트
                close(prev_fd);
            }
            if (i < num_commands - 1) {
                dup2(pipe_fd[1], STDOUT_FILENO);  // 현재 파이프의 쓰기 끝을 표준 출력으로 리다이렉트
            }

            close(pipe_fd[0]);
            close(pipe_fd[1]);

            redirect_io(input_file, output_file);
            execvp(args[0], args);
            perror("명령어 실행 실패");
            exit(1);
        } else {
            if (prev_fd != -1) {
                close(prev_fd);  // 부모 프로세스에서 이전 파이프의 읽기 끝 닫기
            }
            close(pipe_fd[1]);  // 부모 프로세스에서 현재 파이프의 쓰기 끝 닫기
            prev_fd = pipe_fd[0];  // 현재 파이프의 읽기 끝 저장
        }
    }

    if (prev_fd != -1) {
        close(prev_fd);  // 마지막 파이프의 읽기 끝 닫기
    }

    for (int i = 0; i < num_commands; i++) {
        wait(NULL);  // 모든 자식 프로세스 종료 대기
    }
}

// 메인 함수
int main() {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];
    pid_t pid;

    // SIGCHLD 핸들러 등록
    signal(SIGCHLD, sigchld_handler);

    printf("쉘 프로그램을 시작합니다.\n");
    printf("'exit'를 입력하면 종료됩니다.\n");

    while (1) {
        print_prompt();

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break;
        }

        // 줄바꿈 문자 제거
        input[strcspn(input, "\n")] = 0;

        // 빈 입력 처리
        if (strlen(input) == 0) {
            continue;
        }

        // exit 명령어 처리
        if (strcmp(input, "exit") == 0) {
            printf("쉘을 종료합니다.\n");
            break;
        }

        // 파이프 명령어인지 확인
        if (strchr(input, '|') != NULL) {
            execute_piped_commands(input);
        } else {
            // 단일 명령어 처리
            char *input_file = NULL, *output_file = NULL;
            parse_input(input, args);

            for (int i = 0; args[i] != NULL; i++) {
                if (strcmp(args[i], "<") == 0) {
                    input_file = args[i + 1];
                    args[i] = NULL;
                } else if (strcmp(args[i], ">") == 0) {
                    output_file = args[i + 1];
                    args[i] = NULL;
                }
            }

            pid = fork();
            if (pid < 0) {
                perror("fork 실패");
                continue;
            } else if (pid == 0) {
                redirect_io(input_file, output_file);
                execvp(args[0], args);
                perror("명령어 실행 실패");
                exit(1);
            } else {
                wait(NULL);  // 부모 프로세스는 자식 프로세스 종료 대기
            }
        }
    }

    return 0;
}
