/*Brittany Saunders, Samuel Akinmulero
 * June 26th, 2018
 * CS570 Summer 2018
 * Guy Leonard
 * Assignment #3
 */

#include <fcntl.h>
#include <dirent.h>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <iterator>
#include <limits.h>
#include <semaphore.h>
#include <csignal>

#define THREADS 3

//GLOBAL VARIABLES
static sem_t SEM; //Semaphore
static int status;
static int alarm_time = 25; //Default value for countdown timer
static pthread_t threads[THREADS]; //Initialize threads


using namespace std;
namespace
{
    volatile std::sig_atomic_t gSignalStatus;
}


void signal_handler(int signal)
{
    gSignalStatus = signal;
}

//Monitors countdown timer
void *monitor(void *tid) {
    signal(SIGINT, signal_handler); //Initialize signal
    sem_wait(&SEM); //Locks semaphore
    for(int i=alarm_time; i >= 1; i--) { //Loop to update timer every second
        sleep(1);
    }
    raise(SIGINT); //Notifies sibling thread
    sem_post(&SEM); //Unlocks semaphore
    pthread_exit(NULL);
}

//Wall clock
void *wclock(void *tid) {
    sem_wait(&SEM);
    int timelimit = alarm_time;
    time_t currtime;
    struct tm *tinfo; //Time information

    for (int i = 0; i < timelimit; i++) {
        if(gSignalStatus != 0) {
            cout << "ALARM: Process terminating... " << endl;
            sem_post(&SEM);
            pthread_exit(NULL);
        }
        time(&currtime);
        tinfo = localtime(&currtime);
        printf("Current time and date: %s", asctime(tinfo)); //Prints time and date
        sleep(1);
        timelimit++;
    }
}

//Parent thread
void *parent(void *tid){
    sem_wait(&SEM);
    status = pthread_create(&threads[1], NULL, monitor, (void *) 2); //Creates monitor thread
    status = pthread_create(&threads[2], NULL, wclock, (void *) 3); //Creates wall clock thread
    sem_post(&SEM);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    // Initialize semaphore
    sem_init(&SEM, 0, 50);

    //Checks if user put in an argument for the time, if not, alarm_time = 25
    //If condition passes, alarm_time will be set to whatever was put into the argument
    if(argc != 1) {
        char *temp;
        temp = argv[1];
        alarm_time = atoi(temp); //Converts string to integer
    }
    status =  pthread_create(&threads[0], NULL, parent, (void *) 1); //Creates parent thread

    // Error checking
    if (sem_init(&SEM, 0, 50) == -1) {
        printf("Error");
    }

    if (status != 0) {
        printf("Oops. pthread_create returned error code %d", status);
    }

    // Waiting for threads to terminate
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    pthread_join(threads[2], NULL);

    // Destroy semaphore
    sem_destroy(&SEM);
    sleep(2);
    printf("Process has been terminated! (: \n");
    pthread_exit(NULL);
}