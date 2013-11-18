#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string>
#include <math.h>
#include <sys/wait.h>   /* Wait for Process Termination */
#include <iostream>
#include <signal.h>
#include <sys/types.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

 


#define RAND_DIVISOR 1000000000
#define TRUE 1
#define FALSE 0

typedef int Material;
typedef int Product;

#define BUFFER_SIZE 10 // in fact in this particular problem  BUFFER_SIZE is a constant 10

#define MATERIAL_1 1
#define MATERIAL_2 2
#define MATERIAL_3 3

#define PRODUCT_1 1  // produced form MATERIAL_1 and MATERIAL_2 
#define PRODUCT_2 2  // produced from MATERIAL_1 and MATERIAL_3
#define PRODUCT_3 3 // produced form MATERIAL_2 and MATERIAL_3

bool paused = FALSE;

int g_material_1_count = 0;
int g_material_2_count = 0;
int g_material_3_count = 0;

int g_inputBufferDeadlockCounter;
int g_outputQueueDeadlockCounter;

/* the semaphores */
sem_t full, empty;
sem_t tools;

/* The mutex lock */
pthread_mutex_t inputBuffer_mutex;
pthread_mutex_t outputQueue_mutex;


pthread_t tid;       //Thread ID
pthread_attr_t attr; //Set of thread attributes



void show_material_total()
{
	printf("There are %d material_1 have been generated\n",g_material_1_count);
	printf("There are %d material_2 have been generated\n",g_material_2_count);
	printf("There are %d material_3 have been generated\n",g_material_3_count);
}

void show_deadlock()
{
	printf("There occurs %d times deadlock because of inputBuffer\n",g_inputBufferDeadlockCounter);
	printf("There occurs %d times deadlock because of outputQueue\n",g_outputQueueDeadlockCounter);
}

class InputBuffer  //a stack buffer to store the Material
{
private:
	int size;
	int current;
	Material* ary;
	int m1_counter;
	int m2_counter;
	int m3_counter;
public:
	InputBuffer(int s):size(s),current(0),m1_counter(0),m2_counter(0),m3_counter(0)
		{
			ary = new int[size];
			for(int i = 0; i != size; i++)
			{
				ary[i] = -1;   // means no material at that index
			}
		}
		
	~InputBuffer(){delete ary;}
	
	void check_clear()
	{
		if(current < size)
			return;
		else
		{
			int detect[3] = {0,0,0}; // to detect whether there is material_1 or 2 or 3 in the ary
			for(int i = 0; i != size; i++)
			{
				if(ary[i] == MATERIAL_1)
					detect[0] = 1;
				else if (ary[i] == MATERIAL_2)
					detect[1] = 1;
				else if (ary[i] == MATERIAL_3)
					detect[3] = 1;
			}
			// if the ary is full and there is only 2 or 1 kind of material, deadlock may occur,  so discard all the material 
			if((detect[0] + detect[1] + detect[2]) < 3) 
			{
				for(int i = 0; i != size; i++)
					ary[i] = -1;
				g_inputBufferDeadlockCounter++;
			}
		}
	}
	
	int push(Material item)
	{
		check_clear();
		if(current < size)
		{
			if(item == MATERIAL_1)
			{
				m1_counter++;
				++g_material_1_count;
			}
			else if(item == MATERIAL_2)
			{	
				m2_counter++;
				++g_material_2_count;
			}
			else if(item == MATERIAL_3)
			{
				m3_counter++;
				++g_material_3_count;
			}
			else
			{
				printf("********* error: trying to put undefined material into InputBuffer\n \n"); // this line is not necessary can be removed in the future
				return -1;
			}
			for(int i = 0; i != size; i++)
			{
				if(ary[i] == -1) // is empty
				{
					ary[i] = item;
					current++;
					return 0;
				}
			}
			printf("********* push has some errors **************\n \n");
			return -1;
			
		}
		else 
		{ /* Error the buffer is full */
			printf("*********** error: try to push into a full ary\n\n\n");
			return -1;
		}
		
		
	}	
	
	// Material try_pop_m1()
	// {
	// 	if(current > 0)
	// 	{
	// 		for(int i = 0; i != size; i++)
	// 		{
	// 			if(ary[i] == MATERIAL_1)
	// 			{
	// 				ary[i] = -1;
	// 				current--;
	// 				return MATERIAL_1;
	// 			}
	// 		}
	// 		return -1; // means no MATERIAL_1 is found
	// 	}
	// 	else
	// 	{
	// 		printf("*********** error: try to pop from a empty ary\n \n");
	// 		return -1; // error occurs 
	// 	}
	// }
	// 
	// Material try_pop_m3()
	// {
	// 	if(current > 0)
	// 	{
	// 		for(int i = 0; i != size; i++)
	// 		{
	// 			if(ary[i] == MATERIAL_3)
	// 			{
	// 				ary[i] = -1;
	// 				current--;
	// 				return MATERIAL_3;
	// 			}
	// 		}
	// 		return -1; // means no MATERIAL_1 is found
	// 	}
	// 	else
	// 	{
	// 		printf("*********** error: try to pop from a empty ary\n \n");
	// 		return -1; // error occurs 
	// 	}
	// }
	// 
	// Material try_pop_m2()
	// {
	// 	if(current > 0)
	// 	{
	// 		for(int i = 0; i != size; i++)
	// 		{
	// 			if(ary[i] == MATERIAL_2)
	// 			{
	// 				ary[i] = -1;
	// 				current--;
	// 				return MATERIAL_2;
	// 			}
	// 		}
	// 		return -1; // means no MATERIAL_1 is found
	// 	}
	// 	else
	// 	{
	// 		printf("*********** error: try to pop from a empty ary\n \n");
	// 		return -1; // error occurs 
	// 	}
	// }
	
	bool try_get_materials(Product pd, Material* tempHold)
	{
		if(pd == PRODUCT_1)
		{
			int index1 = -1;
			int index2 = -1;
			for(int i = 0; i != size; i++)
			{
				if(ary[i] == MATERIAL_1)
					index1 = i;
				else if (ary[i] == MATERIAL_2)
					index2 = i;
				if((index1 != -1)&&(index2 != -1))
					break;
			}
			if(index1 == -1 || index2 == -1)
				return false;
			else
			{
				ary[index1] = -1;
				ary[index2] = -1;
				current -= 2;
				return true;
			}
		}
		
		else if(pd == PRODUCT_2)
		{
			int index1 = -1;
			int index2 = -1;
			for(int i = 0; i != size; i++)
			{
				if(ary[i] == MATERIAL_1)
					index1 = i;
				else if (ary[i] == MATERIAL_3)
					index2 = i;
				if((index1 != -1)&&(index2 != -1))
					break;
			}
			if(index1 == -1 || index2 == -1)
				return false;
			else
			{
				ary[index1] = -1;
				ary[index2] = -1;
				current -= 2;
				return true;
			}
		}
		
		else if(pd == PRODUCT_3)
		{
			int index1 = -1;
			int index2 = -1;
			for(int i = 0; i != size; i++)
			{
				if(ary[i] == MATERIAL_3)
					index1 = i;
				else if (ary[i] == MATERIAL_2)
					index2 = i;
				
				if((index1 != -1)&&(index2 != -1))
					break;
			}
			if(index1 == -1 || index2 == -1)
				return false;
			else
			{
				ary[index1] = -1;
				ary[index2] = -1;
				current -= 2;
				return true;
			}
		}
		
		else
		{
			printf("************* There is undefined product in try_get_material\n\n");
			return false;
		}
	}
	
	// Material pop()  // won't use in the future
// 	{
// 		Material item = -1; 
// 		if(current > 0) 
// 		{
// 			for(int i =0; i != size; i++)
// 			{
// 				if(ary[i] != -1)
// 				{
// 					item = ary[i];
// 					ary[i] = -1;
// 					--current;
// 					break;
// 				}
// 			}
// 			if(item == MATERIAL_1)
// 				m1_counter--;
// 			else if(item == MATERIAL_2)
// 				m2_counter--;
// 			else if(item == MATERIAL_3)
// 				m3_counter--;
// 			return item;
// 		}
// 		else 
// 		{ /* Error buffer empty */
// 			return -1;
// 		}
// 		
// 	}	
	void showState()
	{
		printf("The state of InputBuffer Now is:\n");
		printf("Totoal: %d\n", current);
		printf("Material_1: %d\n", m1_counter);
		printf("Material_2: %d\n", m2_counter);
		printf("Material_3: %d\n", m3_counter);
	}
	void showAry()
	{
		//pthread_mutex_lock(&inputBuffer_mutex);
		printf("the current Array is: \n");
		for(int i = 0; i != size; i++)
		{
			printf("B: %d\n",ary[i]);
		}
		//pthread_mutex_unlock(&inputBuffer_mutex);
	}
		
};

class OutputQueue // just an unlimited area to store the output of the operator
{
private:
	Product* ary;
	int current;
	int p1_counter;
	int p2_counter;
	int p3_counter;
public:
	OutputQueue():current(0),p1_counter(0),p2_counter(0),p3_counter(0){ary = new int[1000000];}
	~OutputQueue(){delete ary;}
	
	void check_priority_next(Product* priority) // to check produce which product next is best
	{
		if(current == 0) // if there is no product in the queue now, let the generator generate Product_3 first
		{
			priority[0] = PRODUCT_3;
			priority[1] = PRODUCT_2;
		}
		else
		{
			if(ary[current-1] == PRODUCT_1)
			{
				if(p2_counter < p3_counter)
				{
					priority[0] = PRODUCT_2;
					priority[1] = PRODUCT_3;
				}
				else
				{
					priority[0] = PRODUCT_3;
					priority[1] = PRODUCT_2;
				}
			}
			else if(ary[current-1] == PRODUCT_2)
			{
				if(p1_counter < p3_counter)
				{
					priority[0] = PRODUCT_1;
					priority[1] = PRODUCT_3;
				}
				else
				{
					priority[0] = PRODUCT_3;
					priority[1] = PRODUCT_1;
				}
			}
			else if(ary[current-1] == PRODUCT_3)
			{
				if(p2_counter < p1_counter)
				{
					priority[0] = PRODUCT_2;
					priority[1] = PRODUCT_1;
				}
				else
				{
					priority[0] = PRODUCT_1;
					priority[1] = PRODUCT_2;
				}
			}
			else
			{
				printf("********** error: there is undefined product in outputQueue\n");
			}
		}
	}
	
	void push(Product pd)
	{
		ary[current++] = pd;
		if(pd == PRODUCT_1)
			p1_counter++;
		else if (pd == PRODUCT_2)
			p2_counter++;
		else if(pd == PRODUCT_3)
			p3_counter++;
		else
			printf("******* error: in push of outputqueue\n\n\n");
	}
		
	
	bool validate_number(int n1, int n2, int n3)
	{
		if((n1 - n2 > 10) || (n1 - n2 < -10))
			return false;
		if((n1 - n3 > 10) || (n1 - n3 < -10))
			return false;
		if((n2 - n3 > 10) || (n2 - n3 < -10))
			return false;
		return true;
	}
	
	void try_insert_product(Product pd)
	{
		if(pd == ary[current - 1])
		{
			printf("********* error: same product next to each other\n\n\n");
			g_outputQueueDeadlockCounter++;
		}
		else
		{
			if(pd == PRODUCT_1)
			{
				if(validate_number(p1_counter + 1, p2_counter, p3_counter))
					push(pd);
				else
				{
					printf("**********  error invalid number constraint, dealLock occurs, discard this product_%d to make the process continue\n\n\n",pd);
					g_outputQueueDeadlockCounter++;
				}
				
			}
			
			if(pd == PRODUCT_2)
			{
				if(validate_number(p1_counter, p2_counter + 1, p3_counter))
					push(pd);
				else
				{
					printf("***************** error invalid number constraint, dealLock occurs, discard this product_%d to make the process continue\n\n\n",pd);
					g_outputQueueDeadlockCounter++;
				}
				
			}
			
			if(pd == PRODUCT_3)
			{
				if(validate_number(p1_counter, p2_counter, p3_counter + 1))
					push(pd);
				else
				{
					printf("************ error invalid number constraint, dealLock occurs, discard this product_%d to make the process continue\n\n\n",pd);
					g_outputQueueDeadlockCounter++;
				}
				
			}
		}
	}
	
	void showStatus() // remember not to use mutex again around this fuction.
	{
		
		printf("There are totally %d product in outputQueue now.\n",current);
		printf("product_1: %d \n",p1_counter);
		printf("product_2: %d \n",p2_counter);
		printf("product_3: %d \n",p3_counter);
		printf("show input status over\n");
	
	}
	
	void showOutputQueue()
	{
		printf("the outputQueue Now are:\n");
		for(int i = 0; i != current; i++)
		{
			printf("Q: %d\n",ary[i]);
		}
			
	}
	
};


/* the inputbuffer */
InputBuffer inputBuffer(BUFFER_SIZE);

// the outputQueue
OutputQueue outputQueue;



void *generator(void *param); /* the generator thread */
void *operators(void *param); /* the operators thread */
int insert_item(Material item);
int remove_item(Material *item);

void initializeData(int nTools) 
{

	/* Create the inputBuffer_mutex lock */
	pthread_mutex_init(&inputBuffer_mutex, NULL);
	pthread_mutex_init(&outputQueue_mutex, NULL);

	/* Create the full semaphore and initialize to 0 */
	sem_init(&full, 0, 0);

	/* Create the empty semaphore and initialize to BUFFER_SIZE */
	sem_init(&empty, 0, BUFFER_SIZE);
	
	sem_init(&tools,0, nTools/2); // in my strategy, the operator will whether fetch two tools or not fetch any tools, so, the last odd one is useless, by dividing the number of tools by 2, each down and up on semaphore means get or put back two tools
	//printf("there are %d tools \n\n\n\n",nTools/2);

	/* Get the default attributes */
	pthread_attr_init(&attr);


}


/* Producer Thread */
void *generator(void *param) 
{
	int generatorID = *(int*)param;
	Material materialID = generatorID;  
	// the materialID is equal to generatorID, which means the material generated by generator1 is 1(MATERIAL_1),etc 
	while(TRUE) {
		//printf("$$$$$$$ pid is %d $$$$$$$$$$\n", getpid()); //////////////////
		/* sleep for a random period of time */
		//printf("Now in generator%d, sleep for a while\n",generatorID);
		int rNum = (rand() / RAND_DIVISOR)%100;
		sleep(rNum);
		//printf("generator%d finish sleep.\n",generatorID);
		/* acquire the empty lock */
		sem_wait(&empty);
		//printf("generator%d finish to down the semaphore.\n", generatorID);
		/* acquire the inputBuffer_mutex lock */
		pthread_mutex_lock(&inputBuffer_mutex);
		//printf("generator%d enter the critical section.\n", generatorID);

		if(inputBuffer.push(materialID) == -1) {
			printf("************* error:generator%d report error condition\n", generatorID);
		}
		else {
			printf("generator%d produced MATERIAL_%d\n", generatorID, materialID);
		}
		inputBuffer.showAry();
		/* release the inputBuffer_mutex lock */
		pthread_mutex_unlock(&inputBuffer_mutex);
		//printf("generator%d exits the critical section, and try to up the semaphore\n",generatorID);
		/* signal full */
		sem_post(&full);
		//printf("generator%d finish up the semaphore\n",generatorID);
		
	}
	return NULL;
}

/* Consumer Thread */
void *operators(void *param) { 
	int operatorsID = *(int*)param;
	while(TRUE) {
		Material temp_materials[2] = {-1,-1}; // -1 means no material holded 
		//printf("$$$$$$$ pid is %d $$$$$$$$$$\n", getpid()); /////////////
		/* sleep for a random period of time */
		//printf("now in operators%d, sleep for a while\n",operatorsID);
		int rNum = (rand() / RAND_DIVISOR)%100;
		sleep(rNum);
		//printf("operator%d finish sleep\n",operatorsID);
		printf("operator%d try to fetch the tools\n", operatorsID);
		sem_wait(&tools); // aquire two tools
		//printf("There are %d tools in process \n\n\n",(int)tools);
		printf("operator%d fetched the tools\n", operatorsID);
		/* aquire the full lock */
		printf("operators%d try to down get the first material semaphore\n", operatorsID);
		sem_wait(&full); 
		printf("operators%d try to down get the second material semaphore \n", operatorsID);
		sem_wait(&full); 
		printf("operators%d finish getting all the materials semaphores \n",operatorsID);
		/* aquire the inputBuffer_mutex lock */
		pthread_mutex_lock(&inputBuffer_mutex);
		//printf("operators%d enter the critical section\n",operatorsID);
		Product priority[2]; //ask the outputQueue which product to operate first
		outputQueue.check_priority_next(priority);
		if(inputBuffer.try_get_materials(priority[0],temp_materials)) //$$$$$$$$$$$$ maybe don't need temp_materials
		{
			pthread_mutex_lock(&outputQueue_mutex);
			outputQueue.try_insert_product(priority[0]);
			printf("^^^^^^^ operator %d MAY produce the product%d\n",operatorsID,priority[0]);
			pthread_mutex_unlock(&outputQueue_mutex);
			sem_post(&empty);
			sem_post(&empty);
			//sem_post(&tools);
		}
		else if(inputBuffer.try_get_materials(priority[1],temp_materials))
		{
			pthread_mutex_lock(&outputQueue_mutex);
			outputQueue.try_insert_product(priority[1]);
			printf("^^^^^^^ operator %d MAY produce the product%d\n",operatorsID,priority[1]);
			pthread_mutex_unlock(&outputQueue_mutex);
			sem_post(&empty);
			sem_post(&empty);
			//sem_post(&tools);
		}
		else
		{
			sem_post(&full);
			sem_post(&full);
			printf("^^^^^^^ operator_%d put the materials back to the inputBuffer\n\n",operatorsID);
			//sem_post(&tools)
		}
		inputBuffer.showAry();
		pthread_mutex_unlock(&inputBuffer_mutex);
		//printf("operators%d exit the critical section\n",operatorsID);
		/* signal empty */
		printf("operator%d put back the tools\n",operatorsID);
		sem_post(&tools); //release the two tools
		
	}
	return NULL;
}

/* Add an item to the buffer */

/* Remove an item from the buffer */


void execute_child_process(int* args);
void changemode(int);
int  kbhit(void);


int main(int argc, char *argv[])
{

	pid_t child_pid; /* variable to store the child's pid */
	int child_status; /* parent process: child's exit status */
	 
	/* Verify the correct number of arguments were passed in */
	if(argc != 4) 
	{
		fprintf(stderr, "USAGE:./main.out <INT> <INT> <INT>\n");
	}
	

	int mainSleepTime = atoi(argv[1]); /* Time in seconds for main to sleep */
	//int numGeneator = 3; /* Number of generator threads const 3*/  
	int numTools = atoi(argv[2]); // number of tools
	int numOperator = atoi(argv[3]); /* Number of operators threads */
	int args[3] = {mainSleepTime, numTools, numOperator};
	/* Initialize the app */
	initializeData(numTools);
	
	//while(TRUE)
	//{
		child_pid = fork();
		if(child_pid == 0) // child process
		{
			
			//********* maybe only need to execute once, depend on how fork works and when the child process release
			execute_child_process(args);
		}
		else // parent process
		{
			//changemode(1);
			do
			{
				//printf("############## pid is %d $$$$$$$$$$\n \n \n \n\n\n\n\n\n\n\n\n\n\n\n\n", getpid());
				// int ch;
				// 
				// while(TRUE) // ****** just for test need a sleep or counter in fact
				// {
				// 	printf(".");
				// 	if(kbhit())
				// 	{
				// 		ch = getchar();
				// 		if(ch == 'p')
				// 		{
				// 			printf("p is clicked\n");
				// 			paused = !paused;
				// 		}
				// 		else if(ch == 'k')
				// 		{
				// 			printf("k is clicked\n");
				// 			inputBuffer.showState();
				// 		}
				// 		break;
				// 	}
				// 	
				// }
				// changemode(0);
				
				if (!paused) {
					kill(child_pid, SIGCONT);
				} else {
					kill(child_pid, SIGSTOP);
				}
				
				
					
				sleep(1);
			} while (0 == waitpid(child_pid, &child_status, 0));  // maybe don't need this line
		}
		//}
	

	
	
	
	
}

void execute_child_process(int* args)
{
	int mainSleepTime = args[0];/* Time in seconds for main to sleep */
	int numGeneator = 3; /* Number of generator threads  is a const 3 */
	//int numTools = args[1]; // Number of tools
	int numOperator = args[2]; /* Number of operators threads */
	printf("----------------- now in main, before create the generator\n\n\n");
	//printf("$$$$$$$ pid is %d $$$$$$$$$$\n", getpid());///////////////
	/* Create the generator threads */
	static int proCounter = 0;
	for(int i = 0; i < numGeneator; i++) {
		/* Create the thread */
		
		pthread_create(&tid,&attr,generator,&proCounter);
		proCounter++;
		
	}
	//printf("----------------- now in main, finish create the generator, before create the operators\n");
	/* Create the operators threads */
	static int conCounter = 0;
	for(int i = 0; i < numOperator; i++) {
		/* Create the thread */
		
		pthread_create(&tid,&attr,operators,&conCounter);
		conCounter++;
		
	}

	/* Sleep for the specified amount of time in milliseconds */
	//printf("---------------- now in main , finish create operators and before main sleep\n");
	sleep(mainSleepTime);

	/* Exit the program */
	//printf("Exit the program\n");
	show_material_total();
	outputQueue.showStatus();
	show_deadlock();
	outputQueue.showOutputQueue();
	exit(0);
	
}

void changemode(int dir)
{
	static struct termios oldt, newt;
 
	if ( dir == 1 )
	{
		tcgetattr( STDIN_FILENO, &oldt);
		newt = oldt;
		newt.c_lflag &= ~( ICANON | ECHO );
		tcsetattr( STDIN_FILENO, TCSANOW, &newt);
	}
	else
		tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
}
 
int kbhit (void)
{
	struct timeval tv;
	fd_set rdfs;
 
	tv.tv_sec = 0;
	tv.tv_usec = 0;
 
	FD_ZERO(&rdfs);
	FD_SET (STDIN_FILENO, &rdfs);
 
	select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
	return FD_ISSET(STDIN_FILENO, &rdfs);
 
}