/******************************
* Title: line_processor
* Author: Yoon-Orn Chin
* Date: 11/16/2020
* Description: Multi-threaded Producer/Consumer Pipeline
******************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

//Size of the buffer
#define SIZE 50

//Size of each individual line
#define stringSize 1000

//Global Variables
char* buffer_1[SIZE]; //Store input
int count_1 = 0; //Number of items in buffer
int prod_idx_1 = 0; //Index where the input thread will put the next item
int con_idx_1 = 0; //Index where the line seperator will pick up the next item
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER; //Initialize the mutex for buffer 1
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER; //Initialize the condition variable for buffer 1

char* buffer_2[SIZE]; //Line separator 
int count_2 = 0; //Number of items in buffer
int prod_idx_2 = 0; //Index where the line seperator will put the next item
int con_idx_2 = 0; //Index where the plus sign will pick up the next item
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER; //Initialize the mutex for buffer 2
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER; //Initialize the condition variable for buffer 2

char* buffer_3[SIZE]; //sign thread
int count_3 = 0; //Number of items in buffer
int prod_idx_3 = 0; //Index where the plus sign will put the next item
int con_idx_3 = 0; //Index where the output thread will pick up the next item
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER; //Initialize the mutex for buffer 3
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER; //Initialize the condition variable for buffer 3

/***********************
* Function: getInput
* Parameter: args
* Output: User Input or text from txt file
* Why use this: This function is used to take in user input and check every input for 'STOP\n'.
***********************/
void* getInput(void* args) {
	char* userInput;

	//Infinitely running until break
	while(1){

		//Gets user input and stores into userInput variable
		size_t len = 0;
		getline(&userInput, &len, stdin);

		//Store into buffer1
		put_buff_1(userInput);	

		//If userInput is 'STOP\n', break the loop.
		if (strcmp(userInput, "STOP\n") == 0) {
			break;
		}
		
	}
	//printf("thread1 terminating\n");
	return NULL;
}

/***********************
* Function: put_buff_1
* Parameter: Item/User input
* Why use this: This function is used to grab user input and store it into the buffer1 array.
* Source: https://repl.it/@cs344/65prodconspipelinec
***********************/
void put_buff_1(char* item) {
	//Locks mutex before putting the item in the buffer
	pthread_mutex_lock(&mutex_1);

	//Puts item into the buffer
	buffer_1[prod_idx_1] = item;

	//Increments index where next item will be put
	prod_idx_1 = prod_idx_1 + 1;
	count_1++;

	//Signal to consumer that the buffer is no longer empty. It is now full.
	pthread_cond_signal(&full_1);

	//Unlocks mutex
	pthread_mutex_unlock(&mutex_1);
}

/***********************
* Function: get_buff_1
* Output: Return item from buffer1
* Why use this: This function is used to grab the item from buffer1 and return it.
* Source: https://repl.it/@cs344/65prodconspipelinec
***********************/
char* get_buff_1() {
	//Locks the mutex before checking if the buffer contains data to prevent leaking
	pthread_mutex_lock(&mutex_1);

	//While the buffer is empty, wait for it to fill all the way up
	while (count_1 == 0) {
		pthread_cond_wait(&full_1, &mutex_1);
	}

	//Copies whatever is in buffer 1 into item.
	char* item = buffer_1[con_idx_1];

	//Increment the index which the item will be picked up
	con_idx_1 = con_idx_1 + 1;
	count_1--;

	//Unlock mutex
	pthread_mutex_unlock(&mutex_1);

	//Returns item in buffer1
	return item;
}

/***********************
* Function: lineSep
* Parameter: args
* Output: Item without '\n'
* Why use this: This function is used to grab the item from buffer1 and remove all instances of '\n' while replacing it with ' ' and then store into buffer2
***********************/
void* lineSep(void* args) {
	//Infinitely running until break
	while (1) {
		//Store item from buffer1 to item
		char* item = get_buff_1();

		//If item is 'STOP\n', store and then break loop.
		if (strcmp(item, "STOP\n") == 0) {
			put_buff_2(item);
			break;
		}

		//Replace every instance of '\n'
		for (int i = 0; i < strlen(item); i++) {
			if (item[i] == '\n') {
				item[i] = ' ';
			}
		}

		//Store into buffer2
		put_buff_2(item);
	}
	//printf("thread2 terminating\n");
	return NULL;
}

/***********************
* Function: put_buff_2
* Parameter: Item/User input
* Why use this: This function is used to grab user input and store it into the buffer2 array.
* Source: https://repl.it/@cs344/65prodconspipelinec
***********************/
void put_buff_2(char* item) {
	//Lock mutex before putting item in the buffer
	pthread_mutex_lock(&mutex_2);

	//Item into buffer
	buffer_2[prod_idx_2] = item;

	//Increment the index where the next item will be put
	prod_idx_2 = prod_idx_2 + 1;
	count_2++;

	//Signals to the consumer that the buffer is no longer empty
	pthread_cond_signal(&full_2);

	//Unlock the mutex
	pthread_mutex_unlock(&mutex_2);
}

/***********************
* Function: get_buff_2
* Output: Return item from buffer2
* Why use this: This function is used to grab the item from buffer2 and return it.
* Source: https://repl.it/@cs344/65prodconspipelinec
***********************/
char* get_buff_2() {
	//Locks the mutex before checking if the buffer contains data to prevent leaking
	pthread_mutex_lock(&mutex_2);

	//While the buffer is empty, wait for it to fill all the way up
	while (count_2 == 0) {
		pthread_cond_wait(&full_2, &mutex_2);
	}

	//Copies whatever is in buffer 1 into item.
	char* item = buffer_2[con_idx_2];

	//Increment the index which the item will be picked up
	con_idx_2 = con_idx_2 + 1;
	count_2--;

	//Unlock mutex
	pthread_mutex_unlock(&mutex_2);

	//Returns item in buffer2
	return item;
}

/***********************
* Function: changeSign
* Parameter: args
* Output: Item with '^' instead of '++'
* Why use this: This function is used to grab the item from buffer2 and remove all instances of '++' with '^' and store into buffer3.
* Source: Borrowed bits and pieces from my Assignment3
***********************/
void* changeSign(void* args) {
	//Infinitely running until break
	while (1) {
		//Store item from buffer2 to item
		char* item = get_buff_2();

		//Grabbed from assignment3, replaces every instance of ++ with ^
		while (strstr(item, "++") != NULL) {
			char final[stringSize] = "";
			int size = 0;
			for (int i = 0; i < strlen(item); i++) {
				if (item[i] == '+' && item[i + 1] == '+') {
					size = i;
					break;
				}
			}
			strncpy(final, item, size);
			strcat(final, "^");
			strcat(final, (item + size) + 2);
			strcpy(item, final);
		}

		//Store item into buffer3
		put_buff_3(item);

		//If item is 'STOP\n' and break loop.
		if (strcmp(item, "STOP\n") == 0) {
			break;
		}
	}
	//printf("thread3 terminating\n");
	return NULL;
}

/***********************
* Function: put_buff_3
* Parameter: Item/User input
* Why use this: This function is used to grab user input and store it into the buffer3 array.
* Source: https://repl.it/@cs344/65prodconspipelinec
***********************/
void put_buff_3(char* item) {
	//Lock mutex before putting item in the buffer
	pthread_mutex_lock(&mutex_3);

	//Item into buffer
	buffer_3[prod_idx_3] = item;

	//Increment the index where the next item will be put
	prod_idx_3 = prod_idx_3 + 1;
	count_3++;

	//Signals to the consumer that the buffer is no longer empty
	pthread_cond_signal(&full_3);

	//Unlock the mutex
	pthread_mutex_unlock(&mutex_3);
}

/***********************
* Function: get_buff_3
* Output: Return item from buffer3
* Why use this: This function is used to grab the item from buffer3 and return it.
* Source: https://repl.it/@cs344/65prodconspipelinec
***********************/
char* get_buff_3() {
	//Locks the mutex before checking if the buffer contains data to prevent leaking
	pthread_mutex_lock(&mutex_3);

	//While the buffer is empty, wait for it to fill all the way up
	while (count_3 == 0) {
		pthread_cond_wait(&full_3, &mutex_3);
	}

	//Copies whatever is in buffer 1 into item.
	char* item = buffer_3[con_idx_3];

	//Increment the index which the item will be picked up
	con_idx_3 = con_idx_3 + 1;
	count_3--;

	//Unlock mutex
	pthread_mutex_unlock(&mutex_3);

	//Returns item in buffer3
	return item;
}

/***********************
* Function: write_output
* Parameter: args
* Output: Writes every sets of 80 characters into stdout
* Why use this: This function is used to grab the item from buffer3 and store into into a variable. That variable will be processed and output every 80 characters.
***********************/
void* write_output(void* args) {
	//Iterator for buf
	int count = 0;

	//Infinitely running until break
	while (1) {
		//Used to store whatever comes from buffer3 into one large string
		char finalBuffer[stringSize];

		//Used to store whatever is leftover after outputting 80 character values
		char tempBuffer[stringSize];

		//Store 80 characters to output
		char buf[81];

		//Amount of times it outputs 80 characters
		int enter = 0;

		//Grab item from buffer3
		char* item = get_buff_3();

		//If item is equal to 'STOP\n', clear buffers and break the loop.
		if (strcmp(item, "STOP\n") == 0) {
			memset(finalBuffer, '\0', sizeof(finalBuffer));
			memset(buf, '\0', sizeof(buf));
			break;
		}
		
		//Concatenate finalBuffer with new item
		strcat(finalBuffer, item);

		//Size of current buffer
		int size = strlen(finalBuffer);

		//Essentially a value determining how many times 80 goes into the size of the buffer.
		float divis = (float)size / 80.0;

		//Loop through the buffer length	
		for (int i = 0; i < strlen(finalBuffer); i++) {
			//Storing input into buffer
			buf[count] = finalBuffer[i];

			//Increment counter by 1
			count++;

			//If counter == 80, output and then delete buf array
			if (count == 80) {
				printf("%s\n", buf);
				fflush(stdout);
				memset(buf, '\0', sizeof(buf));

				//Reset index for buf
				count = 0;

				//Increment by 1
				enter++;
			}

			//Essentially checking to see if there are any more leftover characters after outputting 80's
			if((divis - enter) > 0 && enter == (int)divis){
				//Iterates through tempBuffer
				int tempIter = 0;

				//Starts from where leftover characters begin all the way until the end
				for (int j = i + 1; j < strlen(finalBuffer); j++) {
					//Copies leftover values from finalBuffer
					tempBuffer[tempIter] = finalBuffer[j];

					//Increment by 1
					tempIter++;
				}

				//Clear finalBuffer for next cycle
				memset(finalBuffer, '\0', sizeof(finalBuffer));

				//Adds all the leftovers back into finalBuffer so next cycle can continue from where it ended.
				strcat(finalBuffer, tempBuffer);

				//Clear tempBuffer for next cycle
				memset(tempBuffer, '\0', sizeof(tempBuffer));

				//Break out of the for loop
				break;
			}
		}
	}
	//printf("thread4 terminating\n");
	return NULL;
}

//Main function
int main() {
	//Declare threads
	pthread_t inputThread, lineSeparator, plusSign, outputThread;

	//Create 4 threads
	pthread_create(&inputThread, NULL, getInput, NULL);
	pthread_create(&lineSeparator, NULL, lineSep, NULL);
	pthread_create(&plusSign, NULL, changeSign, NULL);
	pthread_create(&outputThread, NULL, write_output, NULL);
	
	//Waits until threads are fully terminated
	pthread_join(inputThread, NULL);
	pthread_join(lineSeparator, NULL);
	pthread_join(plusSign, NULL);
	pthread_join(outputThread, NULL);

	//Clears the buffers
	memset(buffer_1, '\0', sizeof(buffer_1));
	memset(buffer_2, '\0', sizeof(buffer_2));
	memset(buffer_3, '\0', sizeof(buffer_3));

	//Return success!
	return EXIT_SUCCESS;
}
