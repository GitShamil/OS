#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHMSZ 100

int main()
{
    key_t key;
    int shmid,semid;
    char *shm, *s;
    struct sembuf sem_lock = {0, -1, SEM_UNDO};
    struct sembuf sem_unlock = {0, 1, SEM_UNDO};

    key = ftok(".", 'm');

           // Создаем семафор
    semid = semget(key, 1, IPC_CREAT | 0666);
    if (semid < 0) {
        perror("semget");
        exit(1);
    }

    // Инициализируем семафор
    if (semctl(semid, 0, SETVAL, 1) < 0) {
        perror("semctl");
        exit(1);
    }

    semop(semid, &sem_lock, 1);

    if ((shmid = shmget(key, SHMSZ, 0666)) < 0) {
        return 1;
    }

    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        return 1;
    }


    for(int i = 0;i < 10,++i){
        s = shm + 4 * i;
        int restored = 0;
        for (int i = 0; i < 4; i++) {
          restored |= ((int)(s+i) & 0xFF) << (i * 8);
        }
        printf("%d\n", random_num);
    }
    semop(semid, &sem_unlock, 1);
}