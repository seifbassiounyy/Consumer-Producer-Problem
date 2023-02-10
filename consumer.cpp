#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <random>
#include <time.h>
#include <unistd.h> 
#include <queue>
#include <vector>
#include <math.h>
#include <cstring>


using namespace std;


struct commodity
{
	char commodity_name[11];
	double commodity_mean;
	double commodity_stanDev;
	double price=0.00;
	double prev_price=0.00;
	double prev_avg=0.00;
	double avg_price=0.00;
	int sleep_interval;
	int size_of_recent=0;
	double recent[6];
	bool high=0;
	bool avg_high=0;
};

struct commodity c[12];


void getTime(){

	struct timespec tp;
	clock_gettime(CLOCK_REALTIME, &tp);
	

	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%d/%m/%Y %X", &tstruct);
	
	std::cerr<<"["<<buf<<".";
	printf("%ld",tp.tv_nsec/1000);
	std::cerr<<"] ";

}


void dashboard()
{
    printf("\e[1;1H\e[2J");
    cout<<"+------------------------------------+\n";
    cout<<"| Currency    |   Price   | AvgPrice |\n";
    cout<<"+------------------------------------+\n";
    for(int i=0; i<10 ;i++)
    {
        cout<<"| "<<c[i].commodity_name;
        printf("\033[%d;15H",i+4);
        cout<<"| ";
        printf("\033[%d;16H",i+4);
        
        if (c[i].price < c[i].prev_price)
        {
        	printf("\033[1;31m%7.2lf↓\033[0m",c[i].price);
        	c[i].high=0;
        }
        
        else if(c[i].price > c[i].prev_price)
        {
        	printf("\033[1;32m%7.2lf↑\033[0m",c[i].price);
        	c[i].high=1;
        }
        
        else if(c[i].price==c[i].prev_price && c[i].price > 0 && c[i].high==1)
        	printf("\033[1;32m%7.2lf↑\033[0m",c[i].price);
        	
        else if(c[i].price==c[i].prev_price && c[i].price > 0 && c[i].high==0)
        	printf("\033[1;31m%7.2lf↓\033[0m",c[i].price);
        		
        else if(c[i].price==0.00)
        	printf("\033[1;36m%7.2lf\033[0m",c[i].price);
        	
        	
        c[i].prev_price=c[i].price;
        
        printf("\033[%d;26H",i+4);
        cout<<" |"<<"   ";
	printf("\033[%d;28H",i+4);
	
        if (c[i].avg_price < c[i].prev_avg)
        {
        	printf("\033[1;31m%7.2lf↓\033[0m",c[i].avg_price);
        	c[i].avg_high=0;
        }
        
        else if (c[i].avg_price > c[i].prev_avg)
        {
        	printf("\033[1;32m%7.2lf↑\033[0m",c[i].avg_price);
        	c[i].avg_high=1;
        }
        
        else if(c[i].avg_price==c[i].prev_avg && c[i].avg_price > 0 && c[i].avg_high==1)
        	printf("\033[1;32m%7.2lf↑\033[0m",c[i].avg_price);
        	
        else if(c[i].avg_price==c[i].prev_avg && c[i].avg_price > 0 && c[i].avg_high==0)
        	printf("\033[1;31m%7.2lf↓\033[0m",c[i].avg_price);
        	
        else if(c[i].price==0)
        	printf("\033[1;36m%7.2lf\033[0m",c[i].avg_price);
        	
        	
        c[i].prev_avg = c[i].avg_price;
        printf("\033[%d;38H",i+4);
        cout<<"|\n";
    }
    cout<<"+------------------------------------+\n";
}

void initialize()
{
	strcpy(c[0].commodity_name, "ALUMINIUM");
	strcpy(c[1].commodity_name, "COPPER");
	strcpy(c[2].commodity_name, "COTTON");
	strcpy(c[3].commodity_name, "CRUDEOIL");
	strcpy(c[4].commodity_name, "GOLD");
	strcpy(c[5].commodity_name, "LEAD");
	strcpy(c[6].commodity_name, "MENTHAOIL");
	strcpy(c[7].commodity_name, "NATURALGAS");
	strcpy(c[8].commodity_name, "NICKEL");
	strcpy(c[9].commodity_name, "SILVER");
	strcpy(c[10].commodity_name, "ZINC");

}


void update(struct commodity temp)
{
	double avg=0;
	for(int i=0; i<11;i++)
	{
		if(strcmp(c[i].commodity_name, temp.commodity_name)==0)
		{
			if(c[i].size_of_recent==0)
			{
				c[i].recent[0]=temp.price;
				c[i].avg_price=temp.price;
				c[i].prev_avg=temp.price;
				c[i].price=temp.price;
				c[i].size_of_recent++;	
			}
			else if(c[i].size_of_recent<4)
			{
				c[i].price=temp.price;
				for(int j=0;j<c[i].size_of_recent;j++)
				{
					avg = avg + c[i].recent[j];
				}
				c[i].recent[c[i].size_of_recent]=temp.price;
				c[i].size_of_recent++;
				avg=avg+c[i].price;
				c[i].avg_price= avg/c[i].size_of_recent;

				
			}
			else if(c[i].size_of_recent == 4)
			{
				c[i].price=temp.price;
				for(int j=0;j<4;j++)
				{
					avg = avg + c[i].recent[j];
				}
		
				for(int j=1; j<4; j++)
				{
					c[i].recent[j-1]=c[i].recent[j];
				}
				c[i].recent[3]=temp.price;
				avg=avg+c[i].price;
				c[i].avg_price= avg/5;
				avg=avg/5;
				avg=0;
			}
		}
	}
}

void sem_wait(struct sembuf operation, int num, int semid)
{
	operation.sem_num = num;
	operation.sem_op = -1;
	operation.sem_flg = 0;
	semop(semid, &operation, 1);
}

void sem_signal(struct sembuf operation, int num, int semid)
{
	operation.sem_num = num;
	operation.sem_op = 1;
	operation.sem_flg = 0;
	semop(semid, &operation, 1);
}

void initialize_semaphores(int semid, int n)
{
	int rc;
	//mutex
	rc = semctl(semid, 0, SETVAL, 1);
	if (rc == -1) {
	    perror("semctl");

	}
	//available
	rc = semctl(semid, 1, SETVAL, 0);
	if (rc == -1) {
	    perror("semctl");

	}
	//empty spots
	rc = semctl(semid, 2, SETVAL, n);
	if (rc == -1) {
	    perror("semctl");
	}

}


void consume(int n)
{
	
	int shmid, shmid1, semid, rc, consumer_index=0;
	int commodity_size=sizeof(commodity);
	int  mutex=0, available=1, empty_spots=2;
	int *producer_index;
	size_t total_size=commodity_size*n;
	struct sembuf operation;
	struct commodity temp;
	struct commodity *buffer;
	key_t key = 0x1234;
	key_t key1 = 0x1235;
	key_t sem_key = ftok(".",'a');
	
	//verify key
	if ((key == -1)||(sem_key == -1)) 
	    {   
		perror("ftok");
		exit(1); 
	    }
	
	//get shared memory
	shmid = shmget(key, total_size, 0666 | IPC_CREAT);
	buffer = (struct commodity*)shmat(shmid, NULL, 0);
	
	shmid1 = shmget(key1, sizeof(int), 0666 | IPC_CREAT);
	producer_index = (int *)shmat(shmid, NULL, 0);
	
	*producer_index = 0;


	if (shmid<0)
	{
		perror("shared memory exists");
	}	
	
	semid = semget(sem_key, 3, IPC_CREAT | IPC_EXCL | 0666);
	
	initialize_semaphores(semid, n);	
		
	while(1)
	{
	
	sem_wait(operation, available, semid);
	
	sem_wait(operation, mutex, semid);
	
	temp=buffer[consumer_index];
	
	consumer_index=(consumer_index+1)%n;
	
	update(temp);
	
	dashboard();
	
	sem_signal(operation, mutex, semid);
	
	sem_signal(operation, empty_spots, semid);

	}
}

int main(int argc, char *argv[])
{

	int n = std::stoi (argv[1]); 

	initialize();
	consume(n);

	return 0;
}


