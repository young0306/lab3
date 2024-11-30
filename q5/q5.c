#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define MAX_LENGTH 1000
#define NUM_SENTENCES 3

// 연습용 문장들
const char* sentences[] = {
    "The quick brown fox jumps over the lazy dog.",
    "Practice makes perfect.",
    "Programming is fun and rewarding."
};

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main() {
    char input[MAX_LENGTH];
    int mistakes = 0;
    int totalChars = 0;
    time_t startTime, endTime;
    double totalSeconds;

    printf("타자 연습 프로그램을 시작합니다.\n");
    printf("각 문장을 보고 그대로 입력하세요.\n\n");

    startTime = time(NULL);

    for (int i = 0; i < NUM_SENTENCES; i++) {
        printf("문장 %d: %s\n", i + 1, sentences[i]);
        printf("입력: ");
        
        fgets(input, MAX_LENGTH, stdin);
        input[strcspn(input, "\n")] = 0;  // 개행문자 제거

        // 오타 수 계산
        int len = strlen(sentences[i]);
        totalChars += len;
        
        for (int j = 0; j < len && j < strlen(input); j++) {
            if (input[j] != sentences[i][j]) {
                mistakes++;
            }
        }
        
        // 길이가 다른 경우 차이만큼 오타로 계산
        if (strlen(input) != len) {
            mistakes += abs(len - strlen(input));
        }

        printf("\n");
    }

    endTime = time(NULL);
    totalSeconds = difftime(endTime, startTime);

    // 분당 타자수 계산 (CPM - Characters Per Minute)
    double cpm = (totalChars / totalSeconds) * 60;

    printf("결과:\n");
    printf("총 오타 수: %d\n", mistakes);
    printf("걸린 시간: %.1f초\n", totalSeconds);
    printf("평균 분당 타자수: %.1f CPM\n", cpm);

    return 0;
}
