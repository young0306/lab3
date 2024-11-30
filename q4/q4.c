#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

void listFilesRecursively(const char *path) {
    struct dirent *entry;
    DIR *dir = opendir(path);
    
    if (dir == NULL) {
        printf("디렉토리를 열 수 없습니다: %s\n", path);
        return;
    }

    // 현재 디렉토리 경로 출력
    printf("\n%s:\n", path);

    // 디렉토리 내용 읽기
    while ((entry = readdir(dir)) != NULL) {
        // . 과 .. 는 건너뛰기
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // 전체 경로 생성
        char fullPath[1024];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);

        struct stat statbuf;
        if (stat(fullPath, &statbuf) == -1)
            continue;

        // 파일 이름 출력
        printf("%s", entry->d_name);
        if (S_ISDIR(statbuf.st_mode))
            printf("/");
        printf("\n");

        // 디렉토리인 경우 재귀적으로 탐색
        if (S_ISDIR(statbuf.st_mode)) {
            listFilesRecursively(fullPath);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    const char *path = argc > 1 ? argv[1] : ".";
    listFilesRecursively(path);
    return 0;
}
