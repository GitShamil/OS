#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>
#include <sys/wait.h>

#define N 5    // number of books

#define MAX_NUMBER_ORDERS 100 // max number of orders


int orders[MAX_NUMBER_ORDERS];

pid_t child_pids[MAX_NUMBER_ORDERS];


char shared_memory_name[] = "semaphore_library";   // name of shared memory
int memory_fd; 					

int semaphore_adress; 	// adress of semaphore

const int password = 2281337;

typedef struct {
    int books[N];
} library;

library* library_of_books;

void closing(int id) {
  semctl(semaphore_adress, 0, IPC_RMID);
  shmctl(memory_fd, IPC_RMID, NULL);
  exit(-1);
}

int main(){
    signal(SIGINT,closing);
    signal(SIGTERM,closing);
    
    struct sembuf acquire = {0, -1, SEM_UNDO};
    struct sembuf release = {0, 1, SEM_UNDO}; 
    
    int num_orders;			// number of orders
    do{
    	printf("write number of users (>=1 and <= %d) \n", MAX_NUMBER_ORDERS);
    	scanf("%d", & num_orders);	// get number of orders
    } while(num_orders < 1 || num_orders > MAX_NUMBER_ORDERS);
    
    srand(22);
    
    for(int i = 0; i < num_orders; ++i){
    	orders[i] = abs(rand() % N);	// make rand orders
    }
    
    // creating semaphore  
    if ((semaphore_adress = semget(password, 1, IPC_CREAT | 0666)) == -1) {
    perror("incorrect creating semaphore");
    exit(1);
    }
    
     if (semctl(semaphore_adress, 0, SETVAL, 1) == -1) {
    perror("incorrect initialize");
    exit(1);
    }
    

    
    // shared memory
    if ((memory_fd = shmget(password, sizeof(library), IPC_CREAT | 0666)) == -1) {
	perror("Can not create shared memory");
	exit(-1);
    } else {
        printf("shared memory created: %s \n", shared_memory_name);
    }
    
    library_of_books = shmat(memory_fd, 0, 0);
    
    int* books = library_of_books->books;
    for(int i = 1; i <= N; ++i){
    	books[i] = i;			// give numbers to each book 
    }
    
    
    // start cycle of booking books
    for(int i = 0; i < num_orders; i++){
    	int result = fork();
    	
    	if (result < 0){
    	    printf("something wrong with fork");
    	    exit(-1);
    	} else if (result == 0){
    	    srand(time(NULL));
      	   // library_of_books = mmap(0, sizeof (library), PROT_READ|PROT_WRITE, MAP_SHARED, memory_fd, 0);
      	    library_of_books = shmat(memory_fd, 0, 0);
      	    int* books = library_of_books->books;
      	    
      	    bool took_book = false;
      	    
      	    while(!took_book){
      	        //waiting queue 
      	  	if(semop(semaphore_adress, &acquire, 1) == -1) {
               	    perror("error wait of posix semaphore");
                    exit(-1);
          	};
            
            	// if book is available 
            	if (books[orders[i]] != -1){
            	    printf("book %d have taken by user %d \n", orders[i], i);
            	    took_book = true;
            	    books[orders[i]] = -1;
            	}
            	
            	if(semop(semaphore_adress, &release, 1) == -1) {
               	    perror("error post of posix semaphore");
                    exit(-1);
          	};	
      	    }
      	    //reading this book
      	    sleep(5 + 20 * ((double)(abs(rand())) / RAND_MAX));
      	    
      	    if(semop(semaphore_adress, &acquire, 1) == -1) {
               	    perror("error wait of posix semaphore");
                    exit(-1);
            };
            //return book to library;
            books[orders[i]] = orders[i];
            printf("book %d have returned by user %d \n", orders[i], i);
            	
            if(semop(semaphore_adress, &release, 1) == -1) {
	       	    perror("error post of posix semaphore");
		    exit(-1);
	    };	
      	    
      	    close(memory_fd);
      	    exit(0);      
    	}
    	sleep(1);
    	child_pids[i] = result;
    }
  for(int i= 0;i<num_orders;++i){
  	int status;
  	waitpid(child_pids[i],&status,0);
  }
  if(semctl(semaphore_adress, 0, IPC_RMID) == -1) {
        perror("incorrect close of reader semaphore");
  };
  if (shmctl(memory_fd, IPC_RMID, NULL) == -1){
    	perror("Cannot close shared memory");
    	exit(-1);
  }
  return 0;
    
}















