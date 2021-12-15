#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#define N 3

int msgid = -1;

typedef struct input_data {
    int num;
} input_data;

struct mbuf {
    long mtype;
};

void* function(void* _data) {
    input_data* data = (input_data*)_data;
    data->num++;
    struct mbuf buf;
    if (msgrcv(msgid, &buf, 0, data->num, 0) < 0) {
        perror("read fail msg");
        exit(errno);
    }
    sleep(rand() % N);
    printf("tread's data %d\n", data->num);
    buf.mtype = data->num + 1;
    if (msgsnd(msgid, &buf, 0, 0) < 0) {
        perror("send fail msg");
        exit(errno);
    }
    pthread_exit(NULL);

}

int main() {
    struct mbuf buf;
    pthread_t thid[N];
    input_data input[N];

    if ((msgid = msgget(IPC_PRIVATE, IPC_CREAT | 0666)) < 0) {
        perror("get fail msg");
        return errno;
    }

    for (int i = 0; i < N; ++i) {
        input[i].num = i;
        if (pthread_create(&thid[i], NULL, function, (void*)(&input[i])) != 0) {
            perror("thread creation fail");
            return errno;
        }
    }
    buf.mtype = 1;
    if (msgsnd(msgid, &buf, 0, 0) < 0) {
        perror("send from master fail");
        return errno;
    }
    if (msgrcv(msgid, &buf, 0, N + 1, 0) < 0) {
        perror("read from master fail");
        return errno;
    }

    for (int j = 0; j < N; ++j) {
        if (pthread_join(thid[j], NULL) != 0) {
            perror("thread join fail");
            return errno;
        }
    }

    return 0;
}