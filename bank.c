#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define TRUE 1

// Account structure
typedef struct {
    int balance;
    pthread_mutex_t lock;
    pthread_cond_t newDeposit;
} Account;

// Initialize the account
Account account = {0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER};

// Function to get balance
int getBalance(Account *account) {
    return account->balance;
}

// Withdraw functionvoid withdraw(Account *account, int amount) {
    pthread_mutex_lock(&account->lock); // 락을 획득합니다.
    if (amount > 10) { // 출금 금액이 너무 큰 경우
        printf("Withdrawal amount is too large: %d\n", amount);
    } else {
        while (account->balance < amount) {
            printf("\t\t\tWait for a deposit\n");
            pthread_cond_wait(&account->newDeposit, &account->lock); // 입금 대기
        }
        account->balance -= amount;
        printf("\t\t\tWithdraw %d\t\t%d\n", amount, getBalance(account));
    }
    pthread_mutex_unlock(&account->lock); // 락을 해제합니다.
}


// Deposit function
void deposit(Account *account, int amount) {
    pthread_mutex_lock(&account->lock); // Acquire the lock
    account->balance += amount;
    printf("Deposit %d\t\t\t\t\t%d\n", amount, getBalance(account));
    
    pthread_cond_signal(&account->newDeposit); // Signal the waiting thread
    pthread_mutex_unlock(&account->lock); // Release the lock
}

// Deposit thread function
void* DepositThread(void *arg) {
    while (TRUE) {
        int amount = (rand() % 10) + 1;
        deposit(&account, amount);
        sleep(1); // Delay to let the withdraw method proceed
    }
    return NULL;
}

// Withdraw thread function
void* WithdrawThread(void *arg) {
    while (TRUE) {
        int amount = (rand() % 10) + 1;
        withdraw(&account, amount);
    }
    return NULL;
}

int main() {
    pthread_t depositThread, withdrawThread;

    // Create deposit and withdraw threads
    pthread_create(&depositThread, NULL, DepositThread, NULL);
    pthread_create(&withdrawThread, NULL, WithdrawThread, NULL);

    // Wait for threads to finish (they won't, since the loops are infinite)
    pthread_join(depositThread, NULL);
    pthread_join(withdrawThread, NULL);

    return 0;
}

