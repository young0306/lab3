#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_INPUT 1024
#define MAX_ARGS 64

// 백그라운드 프로세스를 관리하는 구조체
typedef struct {
    pid_t pid;
    char command[MAX_INPUT];
} BgProcess;

BgProcess bg_processes[100];
int bg_count = 0;

// SIGCHLD 핸들러: 종료된 백그라운드 프로세스를 처리
void sigchld_handler(int signo) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < bg_count; i++) {
            if (bg_processes[i].pid == pid) {
                printf("\n[%d] 완료    %s\n", pid, bg_processes[i].command);
                fflush(stdout);

                // 백그라운드 프로세스 목록에서 제거
                for (int j = i; j < bg_count - 1; j++) {
                    bg_processes[j] = bg_processes[j + 1];
                }
                bg_count--;
                break;
            }
        }
    }
}

// SIGINT 핸들러 (Ctrl-C)
void sigint_handler(int signo) {
    printf("\nCtrl-C 인터럽트가 발생했습니다. 현재 작업을 종료하지 않습니다.\n");
    fflush(stdout);
}

// SIGQUIT 핸들러 (Ctrl-Z)
void sigquit_handler(int signo) {
    printf("\nCtrl-Z 인터럽트가 발생했습니다. 현재 작업을 중지하지 않습니다.\n");
    fflush(stdout);
}

// 사용자 입력 파싱
int parse_input(char *input, char **args) {
    int i = 0;
    int background = 0;
    char *token = strtok(input, " \n");

    while (token != NULL && i < MAX_ARGS - 1) {
        if (strcmp(token, "&") == 0) {
            background = 1;
            break;
        }
        args[i++] = token;
        token = strtok(NULL, " \n");
    }
    args[i] = NULL;
    return background;
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

// 파이프 처리
void handle_pipe(char *command1, char *command2) {
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("파이프 생성 실패");
        exit(1);
    }

    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("fork 실패");
        exit(1);
    }
    if (pid1 == 0) {
        // 첫 번째 프로세스
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[0]);
        close(pipe_fd[1]);

        char *args[MAX_ARGS];
        parse_input(command1, args);
        execvp(args[0], args);
        perror("명령 실행 실패");
        exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("fork 실패");
        exit(1);
    }
    if (pid2 == 0) {
        // 두 번째 프로세스
        dup2(pipe_fd[0], STDIN_FILENO);
        close(pipe_fd[1]);
        close(pipe_fd[0]);

        char *args[MAX_ARGS];
        parse_input(command2, args);
        execvp(args[0], args);
        perror("명령 실행 실패");
        exit(1);
    }

    close(pipe_fd[0]);
    close(pipe_fd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

// 쉘 명령어 구현
void execute_builtin_command(char **args) {
    if (strcmp(args[0], "ls") == 0) {
        execlp("ls", "ls", (char *)NULL);
    } else if (strcmp(args[0], "pwd") == 0) {
        execlp("pwd", "pwd", (char *)NULL);
    } else if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "cd: 경로가 필요합니다.\n");
        } else if (chdir(args[1]) != 0) {
            perror("cd 실패");
        }
    } else if (strcmp(args[0], "mkdir") == 0) {
        execlp("mkdir", "mkdir", args[1], (char *)NULL);
    } else if (strcmp(args[0], "rmdir") == 0) {
        execlp("rmdir", "rmdir", args[1], (char *)NULL);
    } else if (strcmp(args[0], "ln") == 0) {
        if (args[1] && args[2]) {
            execlp("ln", "ln", args[1], args[2], (char *)NULL);
        } else {
            fprintf(stderr, "ln: 사용법: ln <원본 파일> <링크 파일>\n");
        }
    } else if (strcmp(args[0], "cp") == 0) {
        execlp("cp", "cp", args[1], args[2], (char *)NULL);
    } else if (strcmp(args[0], "rm") == 0) {
        execlp("rm", "rm", args[1], (char *)NULL);
    } else if (strcmp(args[0], "mv") == 0) {
        execlp("mv", "mv", args[1], args[2], (char *)NULL);
    } else if (strcmp(args[0], "cat") == 0) {
        execlp("cat", "cat", args[1], (char *)NULL);
    }
}

int main() {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];

    signal(SIGCHLD, sigchld_handler);
    signal(SIGINT, sigint_handler);
    signal(SIGQUIT, sigquit_handler);

    printf("미니 쉘 프로그램 시작\n");
    printf("'exit'을 입력하면 종료됩니다.\n");

    while (1) {
        printf("> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        input[strcspn(input, "\n")] = 0; // 줄바꿈 제거
        if (strcmp(input, "exit") == 0) {
            printf("쉘 종료.\n");
            break;
        }

        int background = parse_input(input, args);

        pid_t pid = fork();
        if (pid == 0) {
            execute_builtin_command(args);
            perror("명령 실행 실패");
            exit(1);
        } else if (!background) {
            waitpid(pid, NULL, 0);
        }
    }

    return 0;
}
