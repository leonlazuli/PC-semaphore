/* buffer.h */
typedef int buffer_item;
#define BUFFER_SIZE 3

/* main.c */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
//#include "buffer.h"

#define RAND_DIVISOR 100000000
#define TRUE 1

class StackBuffer  //a stack buffer to store the material
{
private:
	int size;
	int current;
	int * ary;
public:
	Stack(int s):size(s),current(0){ary = new int[size];}
	~Stack(){delete ary;}
	void push(int n)
	{
		if(current == size)
		{
			cout << "********** error: try to push in to the full buffer ************";
			exit(1);
		}
		ary[current++] = n;
	}	
	int pop()
	{
		if(current == 0)
		{
			cout << "*************error: try to pop from a empty buffer *****************";
			exit(1);
		}
		return ary[--current]; 
	}	
	void showStack()
	{
		cout << "stack: ";
		for(int i = 0; i != current; i++)
			cout << ary[i] << ' ';
		cout << endl;
	}	
};

/* The mutex lock */
pthread_mutex_t mutex;

/* the semaphores */
sem_t full, empty;

/* the buffer */
buffer_item buffer[BUFFER_SIZE];

/* buffer counter */
int counter;

pthread_t tid;       //Thread ID
pthread_attr_t attr; //Set of thread attributes

void *producer(void *param); /* the producer thread */
void *consumer(void *param); /* the consumer thread */
int insert_item(buffer_item item);
int remove_item(buffer_item *item);

void initializeData() {

	/* Create the mutex lock */
	pthread_mutex_init(&mutex, NULL);

	/* Create the full semaphore and initialize to 0 */
	sem_init(&full, 0, 0);

	/* Create the empty semaphore and initialize to BUFFER_SIZE */
	sem_init(&empty, 0, BUFFER_SIZE);

	/* Get the default attributes */
	pthread_attr_init(&attr);

	/* init buffer */
	counter = 0;
}


/* Producer Thread */
void *producer(void *param) 
{
	static buffer_item item = 0;
	int producerID = *(int*)param;
	while(TRUE) {
		/* sleep for a random period of time */
		printf("Now in producer%d, sleep for a while\n",producerID);
		int rNum = (rand() / RAND_DIVISOR)%100;
		sleep(rNum);
		printf("produer%d finish sleep.\n",producerID);

		/* generate a random number */
		item++;

		/* acquire the empty lock */
		sem_wait(&empty);
		printf("produer%d finsh to down the semapare.\n", producerID);
		/* acquire the mutex lock */
		pthread_mutex_lock(&mutex);
		printf("producer%d enter the critical section.\n", producerID);

		if(insert_item(item)) {
			fprintf(stderr, " Producer%d report error condition\n", producerID);
		}
		else {
			printf("producer%d produced item%d\n", producerID, item);
		}
		/* release the mutex lock */
		pthread_mutex_unlock(&mutex);
		printf("produer%d exits the critical section, and try to up the semephore\n",producerID);
		/* signal full */
		sem_post(&full);
		printf("producer%d finish up the semephore",producerID);
	}
	return NULL;
}

/* Consumer Thread */
void *consumer(void *param) {
	buffer_item item;
	int consumerID = *(int*)param;
	while(TRUE) {
		/* sleep for a random period of time */
		printf("now in consumer%d\n, sleep for a while",consumerID);
		int rNum = (rand() / RAND_DIVISOR)%100;
		sleep(rNum);
		printf("comsumer%d finish sleep\n",consumerID);
		

		/* aquire the full lock */
		printf("consumer%d try to down the semaphore\n", consumerID);
		sem_wait(&full);
		printf("consumer%d finish downing the semaphore\n",consumerID);
		/* aquire the mutex lock */
		pthread_mutex_lock(&mutex);
		printf("consumer%d enter the critial section\n",consumerID);
		if(remove_item(&item)) {
			fprintf(stderr, "Consumer%d report error condition\n", consumerID);
		}
		else {
			printf("consumer%d consumed %d\n", consumerID,item);
		}
		/* release the mutex lock */
		pthread_mutex_unlock(&mutex);
		printf("consumer%d exit the critical section\n",consumerID);
		/* signal empty */
		printf("consumer%d try to up the semaphore\n",consumerID);
		sem_post(&empty);
		printf("consumer%d finish uping the semaphore\n",consumerID);
	}
	return NULL;
}

/* Add an item to the buffer */
int insert_item(buffer_item item) {
	/* When the buffer is not full add the item
	and increment the counter*/
	if(counter < BUFFER_SIZE) {
		buffer[counter] = item;
		counter++;
		return 0;
	}
	else { /* Error the buffer is full */
		return -1;
	}
}

/* Remove an item from the buffer */
int remove_item(buffer_item *item) {
	/* When the buffer is not empty remove the item
	and decrement the counter */
	if(counter > 0) {
		*item = buffer[(counter-1)];
		counter--;
		return 0;
	}
	else { /* Error buffer empty */
		return -1;
	}
}

int main(int argc, char *argv[]) {
	/* Loop counter */
	int i;

	/* Verify the correct number of arguments were passed in */
	if(argc != 4) {
		fprintf(stderr, "USAGE:./main.out <INT> <INT> <INT>\n");
	}

	int mainSleepTime = atoi(argv[1]); /* Time in seconds for main to sleep */
	int numProd = atoi(argv[2]); /* Number of producer threads */
	int numCons = atoi(argv[3]); /* Number of consumer threads */

	/* Initialize the app */
	initializeData();

	printf("******* now in main, before create the producer\n");
	/* Create the producer threads */
	static int proCounter = 0;
	for(i = 0; i < numProd; i++) {
		/* Create the thread */
		pthread_create(&tid,&attr,producer,&proCounter);
		proCounter++;
	}
	printf("******** now in main, finish create the producer, before create the consumer\n");
	/* Create the consumer threads */
	static int conCounter = 0;
	for(i = 0; i < numCons; i++) {
		/* Create the thread */
		pthread_create(&tid,&attr,consumer,&conCounter);
		conCounter++;
	}

	/* Sleep for the specified amount of time in milliseconds */
	printf("********* now in main , finish create consumer and before main sleep\n");
	sleep(mainSleepTime);

	/* Exit the program */
	printf("Exit the program\n");
	exit(0);
}