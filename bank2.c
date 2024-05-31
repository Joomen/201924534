#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int balance = 0;
pthread_mutex_t mutex;
pthread_cond_t deposit_cond;

void* DepositThread(void* arg) {
    while (1) {
        int amount = (rand() % 10) + 1;
        pthread_mutex_lock(&mutex);
        balance += amount;
        printf("Deposit %d\t\t\t\t\t%d\n", amount, balance);
        pthread_cond_signal(&deposit_cond);
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
    return NULL;
}

void* WithdrawThread(void* arg) {
    while (1) {
        int amount = (rand() % 10) + 1;
        pthread_mutex_lock(&mutex);
        while (balance < amount) {
            printf("\t\t\tWait for a deposit\n");
            pthread_cond_wait(&deposit_cond, &mutex);
        }
        balance -= amount;
        printf("\t\t\tWithdraw %d\t\t%d\n", amount, balance);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t deposit_thread, withdraw_thread;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&deposit_cond, NULL);

    printf("Thread 1\t\tThread 2\t\tBalance\n");

    pthread_create(&deposit_thread, NULL, DepositThread, NULL);
    pthread_create(&withdraw_thread, NULL, WithdrawThread, NULL);

    pthread_join(deposit_thread, NULL);
    pthread_join(withdraw_thread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&deposit_cond);

    return 0;
}
