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


/* producer function.
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
 */
void* producer(void *pthread){
    while (item_counter < MAX_ITEMS){
        //Lock buffer for thread
        //Check if buffer is full, if it is wait.
        while(((in + 1) % BUFFER_SIZE) == out){
            sem_wait(&full);
        }
        pthread_mutex_lock(&lock);
        //Insert value into buffer
        int valtoadd = rand() % 100;
        buffer[in].value = valtoadd;
        in = ((in + 1) % BUFFER_SIZE);
        printf("Producer%d adds #%d to slot %d.\n",
               (int)pthread,valtoadd,in);
        //unlock the buffer for other processes
        pthread_mutex_unlock(&lock);
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
 
 */
void* consumer(void *pthread){
    item value;
    while(item_counter < MAX_ITEMS){
        while(out == in){
            sem_wait(&empty);
        }
        pthread_mutex_lock(&lock);
        value.value = buffer[out].value;
        buffer[out].value = 0;
        out = ((out + 1) % BUFFER_SIZE);
        printf("Consumer%d removes #%d from slot %d.\n",
               (int)pthread,value.value,out);
        item_counter++;
        pthread_mutex_unlock(&lock);
        sem_post(&full);
    }
    //probably not right
    return 0;
}



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
        NUM_CONSUMERS = atoi(argv[1]);
        NUM_PRODUCERS = atoi(argv[2]);
        MAX_ITEMS = atoi(argv[3]);
        BUFFER_SIZE = atoi(argv[4]);
    } else {
        printf("ERROR: Needs 0 or 4 integer parameters.\n");
        printf("Provide parameters in the following order: Consumers, Producers, Items, Buffer Size.\n");
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
