#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <fcntl.h>

#define SHM_SIZE 4096
#define SEM_KEY 12345

// 세마포어 유니온 정의
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

// 세마포어 초기화 함수
int init_semaphore(int sem_id, int value) {
    union semun sem_union;
    sem_union.val = value;
    return semctl(sem_id, 0, SETVAL, sem_union);
}

// 세마포어 P 연산 (잠금)
void sem_wait(int sem_id) {
    struct sembuf sb = {0, -1, 0};
    semop(sem_id, &sb, 1);
}

// 세마포어 V 연산 (해제)
void sem_signal(int sem_id) {
    struct sembuf sb = {0, 1, 0};
    semop(sem_id, &sb, 1);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("사용법: %s <원본파일> <대상파일>\n", argv[0]);
        exit(1);
    }

    // 공유 메모리 생성
    int shm_id = shmget(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("공유 메모리 생성 실패");
        exit(1);
    }

    // 세마포어 생성
    int sem_id = semget(SEM_KEY, 2, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("세마포어 생성 실패");
        exit(1);
    }

    // 세마포어 초기화 (read_sem = 1, write_sem = 0)
    init_semaphore(sem_id, 1);  // read_sem
    init_semaphore(sem_id + 1, 0);  // write_sem

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork 실패");
        exit(1);
    }

    // 공유 메모리 연결
    char *shared_memory = (char *)shmat(shm_id, NULL, 0);
    if (shared_memory == (char *)-1) {
        perror("공유 메모리 연결 실패");
        exit(1);
    }

    if (pid > 0) {  // 부모 프로세스 (읽기)
        int fd = open(argv[1], O_RDONLY);
        if (fd == -1) {
            perror("원본 파일 열기 실패");
            exit(1);
        }

        int bytes_read;
        sem_wait(sem_id);  // 초기 잠금

        // 한 번만 읽기
        bytes_read = read(fd, shared_memory, SHM_SIZE - sizeof(int));
        if (bytes_read == -1) {
            perror("파일 읽기 실패");
            exit(1);
        }

        // 읽은 크기 저장
        *((int *)(shared_memory + SHM_SIZE - sizeof(int))) = bytes_read;

        sem_signal(sem_id + 1);  // 쓰기 프로세스에게 신호

        close(fd);
        wait(NULL);
    }
    else {  // 자식 프로세스 (쓰기)
        int fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd == -1) {
            perror("대상 파일 열기 실패");
            exit(1);
        }

        sem_wait(sem_id + 1);  // 읽기 완료 대기

        // 읽은 크기 확인
        int bytes_to_write = *((int *)(shared_memory + SHM_SIZE - sizeof(int)));

        if (bytes_to_write > 0) {
            // 한 번만 쓰기
            if (write(fd, shared_memory, bytes_to_write) == -1) {
                perror("파일 쓰기 실패");
                exit(1);
            }
        }

        sem_signal(sem_id);  // 읽기 프로세스에게 신호

        close(fd);
    }

    // 공유 메모리 해제
    shmdt(shared_memory);
    if (pid > 0) {
        shmctl(shm_id, IPC_RMID, NULL);
        semctl(sem_id, 0, IPC_RMID);
    }

    return 0;
}
