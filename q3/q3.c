#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>

void create_and_write_file() {
    // 파일 생성 및 쓰기
    int fd = open("test.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("파일 생성 실패");
        return;
    }
    
    char *text = "Hello, File I/O!\n";
    write(fd, text, strlen(text));
    close(fd);
    printf("1. 파일 생성 및 쓰기 완료\n");
}

void read_file() {
    // 파일 읽기
    int fd = open("test.txt", O_RDONLY);
    if (fd == -1) {
        perror("파일 열기 실패");
        return;
    }
    
    char buffer[100];
    int bytes_read = read(fd, buffer, sizeof(buffer)-1);
    buffer[bytes_read] = '\0';
    close(fd);
    
    printf("2. 파일 내용: %s", buffer);
}

void file_info() {
    // 파일 정보 확인
    struct stat file_stat;
    if (stat("test.txt", &file_stat) == -1) {
        perror("파일 정보 읽기 실패");
        return;
    }
    
    printf("3. 파일 정보:\n");
    printf("   크기: %ld bytes\n", file_stat.st_size);
    printf("   권한: %o\n", file_stat.st_mode & 0777);
    printf("   수정 시간: %s", ctime(&file_stat.st_mtime));
}

void list_directory() {
    // 디렉토리 내용 나열
    DIR *dir = opendir(".");
    if (dir == NULL) {
        perror("디렉토리 열기 실패");
        return;
    }
    
    struct dirent *entry;
    printf("4. 현재 디렉토리 내용:\n");
    while ((entry = readdir(dir)) != NULL) {
        struct stat file_stat;
        if (stat(entry->d_name, &file_stat) == 0) {
            printf("   %s", entry->d_name);
            if (S_ISDIR(file_stat.st_mode)) {
                printf("/");
            }
            printf("\n");
        }
    }
    closedir(dir);
}

int main() {
    printf("파일 및 디렉토리 조작 예제 시작\n\n");
    
    create_and_write_file();
    read_file();
    file_info();
    list_directory();
    
    return 0;
} 
