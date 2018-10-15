/*Samuel Akinmulero, Brittany Saunders
 * May 23rd, 2018
 * CS570 Summer 2018
 * Guy Leonard
 * Assignment #1
 */

#include <iostream>
#include <semaphore.h>
#include <signal.h>
#include <pthread.h>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <errno.h>
#include <time.h>

// Declare and initialize variables
using namespace std;

#define THREADS 4
static char rankList[13] = {'A', '2', '3', '4', '5', '6', '7', '8', '9', '10', 'J', 'Q', 'K'}; // char array list for face values
static string rank;
static string suit;
static int pid;
sem_t FLAG; //Semaphore
int status;
unsigned int value = 55;
static FILE *filep; //File pointer to filep

//Use semaphore calls here

//Diamond thread function
void *diamond(void *tid) {
    for(int i = 0; i< 13; i++) {
        sem_wait(&FLAG); // locks semaphore
        value--; // decrements semaphore value
        usleep(125000); // sleep for specified amount of time
        suit = "Diamond";
        rank = rankList[i]; // gets proper face value from rank array
        filep = fopen("STACK.txt", "a"); //opens file
        // Print thread to STACK.txt
        if (i == 9)
            fprintf(filep, "Thread %d : %s %0i \r\n", tid, suit.c_str(), 10);
        else
            fprintf(filep, "Thread %d : %s %s \r\n", tid, suit.c_str(), rank.c_str());
        fclose(filep); //closes file
        // Update console
        printf("Thread %d is running \r\n", tid);
        value++;
    }
    // Close semaphore
    sem_post(&FLAG);
    pthread_exit(NULL);
}

// Club thread function
void *club(void *tid) {
    for(int i = 0; i<13;i++) {
        sem_wait(&FLAG);
        value--;
        usleep(250000);
        suit = "Club";
        rank = rankList[i];
        filep = fopen("STACK.txt", "a");
        if (i == 9)
            fprintf(filep, "Thread %d : %s %0i \r\n", tid, suit.c_str(), 10);
        else
            fprintf(filep, "Thread %d : %s %s \r\n", tid, suit.c_str(), rank.c_str());
        fclose(filep);
        printf("Thread %d is running \n", tid);
        value++;
    }
    sem_post(&FLAG);
    pthread_exit(NULL);
}

// Hearts thread function
void *hearts(void *tid) {
    for(int i=0;i<13;i++) {
        sem_wait(&FLAG);
        value--;
        usleep(500000);
        suit = "Heart";
        rank = rankList[i];
        filep = fopen("STACK.txt", "a");
        if (i == 9)
            fprintf(filep, "Thread %d : %s %0i \r\n", tid, suit.c_str(), 10);
        else
            fprintf(filep, "Thread %d : %s %s \r\n", tid, suit.c_str(), rank.c_str());
        fclose(filep);
        printf("Thread %d is running \r\n", tid);
        value++;
    }
    sem_post(&FLAG);
    pthread_exit(NULL);
}

// Spades thread function
void *spades(void *tid) {
    for(int i = 0; i < 13;i++) {
        sem_wait(&FLAG);
        value--;
        usleep(750000);
        suit = "Spade";
        rank = rankList[i];
        filep = fopen("STACK.txt", "a");
        if (i == 9)
            fprintf(filep, "Thread %d : %s %0i \r\n", tid, suit.c_str(), 10);
        else
            fprintf(filep, "Thread %d : %s %s \r\n", tid, suit.c_str(), rank.c_str());
        fclose(filep);
        printf("Thread %d is running \r\n", tid);
        value++;
    }
    sem_post(&FLAG);
    pthread_exit(NULL);
}

//Use POSIX system calls
int main(int argc, char *argv[]) {

    // Opens file for PID
    filep = fopen("STACK.txt","a");

    // Gets PID and prints to STACK.txt
    pid = getpid();
    fprintf(filep, "PID: %0i \r\n", pid);

    // Close file after PID is entered
    fclose(filep);

    // Initialize semaphore
    sem_init(&FLAG, 0, value);
    // Initialize threads
    pthread_t threads[THREADS];

    // Create threads
    status =  pthread_create(&threads[0], NULL, diamond, (void *) 1);
    status = pthread_create(&threads[1], NULL, club, (void *) 2);
    status = pthread_create(&threads[2], NULL, hearts, (void *) 3);
    status =  pthread_create(&threads[3], NULL, spades, (void *) 4);

    // Error checking
    if (sem_init(&FLAG, 0, value) == -1) {
        printf("Error");
    }

    if (status != 0) {
        printf("Oops. pthread_create returned error code %d", status);
    }

    // Waiting for threads to terminate
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    pthread_join(threads[2], NULL);
    pthread_join(threads[3], NULL);

    // Destroy semaphore
    sem_destroy(&FLAG);
    // Graceful exit
    printf("FLAG destroyed! Process has finished! (:");
    pthread_exit(NULL);

}