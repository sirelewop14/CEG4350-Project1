/*
 Rhys Powell
 CEG4350 Summer 2016
 Project 1: Producer / Consumer Problem
 */



#include <stdio.h> //Printing statements
#include <semaphore.h> //For semaphores
#include <pthread.h> //For pthreads
#include <stdlib.h> //For rand() function

//Create the buffer object that will hold data.
typedef struct{
    int value;
} item;

//Set up indices.
int in = 0;
int out = 0;

//Name variables for user input (or defaults).
int BUFFER_SIZE;
int NUM_PRODUCERS;
int NUM_CONSUMERS;
int MAX_ITEMS;
int item_counter = 0;

//Create a pointer to a buffer for global access for all threads.
item *buffer;

//Empty and full semaphores for thread access.
sem_t empty;
sem_t full;

//Mutex for thread locking
pthread_mutex_t lock;


/* Producer function.
 * This function is to be called with a pthread create statement.
 * It Receives a value that identifies the thread with a number.
 * This function enters a neverending loop and checks the buffer to see if
 it is full. If it is full, the producer will wait. After determining the
 buffer is not full, the producer will lock the buffer and attempt to insert
 an randomly generated integer between 0-100.
 * After inserting the number, it will print out the insertion statement
 and then unlock the mutex, increment the emty semaphore to signal that the
 buffer has an item in it, and then start over waiting for the buffer to be
 ready again.
 
 @param void *pthread - This parameter is a unique ID for the thread itself.
                      - This is required by the pthread library.
 @return void* - returns a pointer of any type, this is required by the pthread library.
 */
void* producer(void *pthread){
    while (item_counter < MAX_ITEMS){
        //Lock buffer for thread
        //Check if buffer is full, if it is wait.
        while(((in + 1) % BUFFER_SIZE) == out){
            sem_wait(&full);
        }
        pthread_mutex_lock(&lock); //ENTRY SECTION
        //Insert value into buffer
        //BEGIN CRITICAL SECTION
        int valtoadd = rand() % 100;
        buffer[in].value = valtoadd;
        in = ((in + 1) % BUFFER_SIZE);
        printf("Producer%d adds #%d to slot %d.\n",
               (int)pthread,valtoadd,in);
        //unlock the buffer for other processes
        //END CRITICAL SECTION
        pthread_mutex_unlock(&lock);//EXIT SECTION
        //Increment semaphore index
        sem_post(&empty);
    }
    //probably not right
    return 0;
}

/* The Consumer Function
 * This function is to be called with a pthread create statement.
 * It Receives a value that identifies the thread with a number.
 * This function enters a neverending loop and checks the buffer to see if
 it is empty. If it is empty, the consumer will wait. After determining the
 buffer is not empty, the consumer will lock the buffer and attempt to remove
 the item that is in the last position in the buffer indicated by the variable
 "out."
 *Once the buffer slot is emptied, the consumer prints out the value removed,
 and then set that slot to 0. From that point it unlocks the buffer and increments
 the full semaphore. It will then loop again and wait for the buffer to be full again.
 
 @param void *pthread - This parameter is a unique ID for the thread itself.
                      - This is required by the pthread library.
 @return void* - returns a pointer of any type, this is required by the pthread library.
 */
void* consumer(void *pthread){
    item value;
    while(item_counter < MAX_ITEMS){
        //If buffer is empty, nothing to consume so wait.
        while(out == in){
            sem_wait(&empty);
        }
        pthread_mutex_lock(&lock); //ENTRY SECTION
        //BEGIN CRITICAL SECTION
        //Remove value from buffer and set buffer to 0 (empty)
        value.value = buffer[out].value;
        buffer[out].value = 0;
        out = ((out + 1) % BUFFER_SIZE);
        printf("Consumer%d removes #%d from slot %d.\n",
               (int)pthread,value.value,out);
        item_counter++;
        //END CRITICAL SECTION
        //Unlock the buffer
        pthread_mutex_unlock(&lock);//EXIT SECTION
        sem_post(&full);
    }
    //probably not right
    return 0;
}

/*
 The main method.
 Accepts input from the user at the command line, if any less than 4 integer values are inserted,
 the program runs with the set default values.
 
 This function also creates a variable number of producers and consumers based on input and 
 also creates the buffer based on user input size. After creating and running the threads,
 it joins them all up at the end and exits the application.
 
 @param int argc - integer value that shows how many paramters were passed to the program.
                 - Always has at least a 1, since the program itself is a parameter.
 @param char **argv - char array that contains all the parameters passed to the program.
 */

int main (int argc, char **argv){
    void * threadID = 0;
    
    //Checks for user input.
    //Assumes that if user input is desired, user will input ALL variables. Not just one or two.
    //Variables must be entered in the order defined in the assignment.
    if (argc == 1){
        NUM_CONSUMERS = 2;
        NUM_PRODUCERS = 2;
        MAX_ITEMS = 20;
        BUFFER_SIZE = 10;
    } else if (argc == 5){
        BUFFER_SIZE = atoi(argv[1]);
        NUM_PRODUCERS = atoi(argv[2]);
        NUM_CONSUMERS = atoi(argv[3]);
        MAX_ITEMS = atoi(argv[4]);
    } else {
        printf("ERROR: Needs 0 or 4 integer parameters.\n");
        printf("Provide parameters in the following order: Buffer Size, Producers, Consumers, Items.\n");
        return 0;
    }
    
    //Instantiate the buffer for the global pointer.
    buffer = malloc(BUFFER_SIZE);
    
    //initialize the mutex to controll access to the buffer.
    //initialized with default values (NULL).
    pthread_mutex_init(&lock, NULL);
    
    //Create thread objects for producers and consumers.
    //And then create the actual threads.
    //The final parameter in each declaration is a "unique" number.
    //which identifies the individual threads.
    pthread_t producer_t[NUM_PRODUCERS];
    for (int i =0; i < NUM_PRODUCERS; i++){
        pthread_create(&producer_t[i], NULL, producer, threadID+i);
    }
    
    pthread_t consumer_t[NUM_CONSUMERS];
    for (int i = 0; i < NUM_CONSUMERS; i++){
        pthread_create(&consumer_t[i], NULL, consumer, threadID+i);
    }
    
    //Clean up all the threads
    //Loops through all the threads generated in previous step.
    for (int i = 0; i < NUM_PRODUCERS; i++){
        pthread_join(producer_t[i],NULL);
    }
    
    for (int i = 0; i < NUM_CONSUMERS; i++){
        pthread_join(consumer_t[i],NULL);
    }
    
    return 0;
}
