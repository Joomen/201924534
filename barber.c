#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>

#define MAX_CUSTOMERS 25
#define NUM_BARBERS 3

typedef struct {
    int numChairs;
    int numCustomers;
    int customerCount;
    int servedCustomers;
    bool done; 
} SharedData;

#define SEM_WAITING_ROOM "/waiting_room"
#define SEM_BARBER_CHAIR "/barber_chair"
#define SEM_BARBER_PILLOW "/barber_pillow"
#define SEM_SEAT_BELT "/seat_belt"

void *customer(void *arg);
void *barber(void *arg);
void randwait(int secs);

int main(int argc, char *argv[]) {
    int i, shmid;
    pid_t pid;
    SharedData *sharedData;
    sem_t *waitingRoom, *barberChair, *barberPillow, *seatBelt;

    if (argc != 4) {
        printf("Usage: %s <num_customers> <num_chairs> <rand_seed>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    shmid = shmget(IPC_PRIVATE, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    sharedData = (SharedData *) shmat(shmid, NULL, 0);
    sharedData->numChairs = atoi(argv[2]);
    sharedData->numCustomers = atoi(argv[1]);
    sharedData->customerCount = 0;
    sharedData->servedCustomers = 0;
    sharedData->done = false; 

    waitingRoom = sem_open(SEM_WAITING_ROOM, O_CREAT, 0666, sharedData->numChairs);
    barberChair = sem_open(SEM_BARBER_CHAIR, O_CREAT, 0666, NUM_BARBERS);
    barberPillow = sem_open(SEM_BARBER_PILLOW, O_CREAT, 0666, 0);
    seatBelt = sem_open(SEM_SEAT_BELT, O_CREAT, 0666, 0);

    srand(atol(argv[3]));

    for (i = 0; i < NUM_BARBERS; i++) {
        pid = fork();
        if (pid == 0) {
            barber(sharedData);
            exit(EXIT_SUCCESS);
        }
    }

    for (i = 0; i < sharedData->numCustomers; i++) {
        pid = fork();
        if (pid == 0) {
            customer(sharedData);
            exit(EXIT_SUCCESS);
        }
    }

	while (wait(NULL) > 0) {
    	if (sharedData->servedCustomers == sharedData->numCustomers) {
        	sharedData->done = true; 
        	for (i = 0; i < NUM_BARBERS; i++) {
        	    printf("The barber is going home for the day.\n");
    	    }
    	}
	}

    sem_unlink(SEM_WAITING_ROOM);
    sem_unlink(SEM_BARBER_CHAIR);
    sem_unlink(SEM_BARBER_PILLOW);
    sem_unlink(SEM_SEAT_BELT);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}

void *customer(void *arg) {
    SharedData *sharedData = (SharedData *) arg;
    sem_t *waitingRoom, *barberChair, *barberPillow, *seatBelt;
    int customerNum;

    waitingRoom = sem_open(SEM_WAITING_ROOM, 0);
    barberChair = sem_open(SEM_BARBER_CHAIR, 0);
    barberPillow = sem_open(SEM_BARBER_PILLOW, 0);
    seatBelt = sem_open(SEM_SEAT_BELT, 0);

    customerNum = sharedData->customerCount;
    sharedData->customerCount++;

    printf("Customer %d leaving for barber shop.\n", customerNum);
    randwait(5);
    printf("Customer %d arrived at barber shop.\n", customerNum);

    sem_wait(waitingRoom);
    printf("Customer %d entering waiting room.\n", customerNum);

    sem_wait(barberChair);
    sem_post(waitingRoom);

    printf("Customer %d waking the barber.\n", customerNum);
    sem_post(barberPillow);

    sem_wait(seatBelt);

    sem_post(barberChair);
    printf("Customer %d leaving barber shop.\n", customerNum);

    sharedData->servedCustomers++;

    sem_close(waitingRoom);
    sem_close(barberChair);
    sem_close(barberPillow);
    sem_close(seatBelt);
}

void *barber(void *arg) {
    SharedData *sharedData = (SharedData *) arg;
    sem_t *waitingRoom, *barberChair, *barberPillow, *seatBelt;

    waitingRoom = sem_open(SEM_WAITING_ROOM, 0);
    barberChair = sem_open(SEM_BARBER_CHAIR, 0);
    barberPillow = sem_open(SEM_BARBER_PILLOW, 0);
    seatBelt = sem_open(SEM_SEAT_BELT, 0);

    while (1) { 
        printf("The barber is sleeping\n");
        sem_wait(barberPillow);

        printf("The barber is cutting hair\n");
        randwait(3);
        printf("The barber has finished cutting hair.\n");

        sem_post(seatBelt);
    }

    sem_close(waitingRoom);
    sem_close(barberChair);
    sem_close(barberPillow);
    sem_close(seatBelt);
}


void randwait(int secs) {
    int len;
    len = (int) ((drand48() * secs) + 1);
    sleep(len);
}
