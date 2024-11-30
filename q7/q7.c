#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>

// 파일의 권한을 문자열로 변환하는 함수
void get_permissions(mode_t mode, char *perms) {
    sprintf(perms, "%c%c%c%c%c%c%c%c%c%c",
        S_ISDIR(mode) ? 'd' : '-',
        mode & S_IRUSR ? 'r' : '-',
        mode & S_IWUSR ? 'w' : '-',
        mode & S_IXUSR ? 'x' : '-',
        mode & S_IRGRP ? 'r' : '-',
        mode & S_IWGRP ? 'w' : '-',
        mode & S_IXGRP ? 'x' : '-',
        mode & S_IROTH ? 'r' : '-',
        mode & S_IWOTH ? 'w' : '-',
        mode & S_IXOTH ? 'x' : '-'
    );
}

// 파일 크기를 사람이 읽기 쉬운 형태로 변환
void format_size(off_t size, char *buf) {
    if (size < 1024)
        sprintf(buf, "%5ld", size);
    else if (size < 1024 * 1024)
        sprintf(buf, "%4ldK", size / 1024);
    else if (size < 1024 * 1024 * 1024)
        sprintf(buf, "%4ldM", size / (1024 * 1024));
    else
        sprintf(buf, "%4ldG", size / (1024 * 1024 * 1024));
}

int main() {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    struct passwd *pwd;
    struct group *grp;
    char perms[11];
    char date_str[13];
    char size_str[10];
    long total_blocks = 0;

    // 현재 디렉토리 열기
    dir = opendir(".");
    if (dir == NULL) {
        perror("디렉토리를 열 수 없습니다");
        return 1;
    }

    // 전체 블록 수 계산
    while ((entry = readdir(dir)) != NULL) {
        if (stat(entry->d_name, &file_stat) == 0) {
            total_blocks += file_stat.st_blocks / 2;  // 512바이트 블록을 1K 블록으로 변환
        }
    }
    printf("total %ld\n", total_blocks);

    // 디렉토리 포인터 처음으로 되돌리기
    rewinddir(dir);

    // 각 파일의 정보 출력
    while ((entry = readdir(dir)) != NULL) {
        if (stat(entry->d_name, &file_stat) == 0) {
            // 권한 문자열 얻기
            get_permissions(file_stat.st_mode, perms);

            // 소유자와 그룹 정보 얻기
            pwd = getpwuid(file_stat.st_uid);
            grp = getgrgid(file_stat.st_gid);

            // 시간 정보 포맷팅
            strftime(date_str, sizeof(date_str), "%b %d %H:%M", 
                    localtime(&file_stat.st_mtime));

            // 파일 크기 포맷팅
            format_size(file_stat.st_size, size_str);

            // 정보 출력
            printf("%s %2ld %-8s %-8s %8s %s %s\n",
                perms,
                file_stat.st_nlink,
                pwd ? pwd->pw_name : "unknown",
                grp ? grp->gr_name : "unknown",
                size_str,
                date_str,
                entry->d_name);
        }
    }

    closedir(dir);
    return 0;
} 
