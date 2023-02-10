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


double getNormalDist(struct commodity c){

	default_random_engine generator;
  	normal_distribution<double> distribution{c.commodity_mean,c.commodity_stanDev};
  	
  	double newPrice;
	int n = rand() % 50 + 1;
	for (int i=0; i<n; ++i) {
	    newPrice = distribution(generator);
	}
  	std::cout << newPrice << std::endl;
	return newPrice;

}

void getTime(){

	struct timespec tp;
	clock_gettime(CLOCK_REALTIME, &tp);
	
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%d/%m/%Y %X", &tstruct);
	
	cerr<<"["<<buf<<".";
	printf("%3ld",tp.tv_nsec/1000000);
	cerr<<"] ";

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


void produce(struct commodity c, int n)
{
	int shmid, semid, shmid1;
	int commodity_size=sizeof(commodity);
	int  mutex=0, available=1, empty_spots=2;
	int *index;
	size_t total_size=commodity_size*n;
	struct sembuf operation;
	struct commodity temp;
	struct commodity *buffer;
	key_t key = 0x1234;
	key_t key1 = 0x1235;
	key_t sem_key = ftok(".",'a');
	
	//getTime();
	//cout<<": total_size"<< total_size<<"\n";
		
	
	if ((key == -1)||(sem_key == -1)) 
	    {   
		perror("ftok");
		exit(1); 
	    }
	
	semid = semget(sem_key, 3, 0666);
	
	if (semid==-1)
	{
	    perror("Semaphore denied!");
		cout<<errno<<"\n";
		exit(1);
	}
	
	
	shmid = shmget(key, total_size, 0666 | IPC_CREAT);
	buffer = (struct commodity*) shmat(shmid, NULL, 0);
	
	shmid1 = shmget(key1, sizeof(int), 0666 | IPC_CREAT);
	index = (int*) shmat(shmid1, NULL, 0);
	

	
	if(buffer == NULL)
	{
		getTime();
		cout<<": Pointer is empty\n";
		exit(1);
	}
	
	
	getTime();
	cout<<c.commodity_name<<": Waiting on empty_spots\n";
	sem_wait(operation, empty_spots, semid);
	
	getTime();
	cout<<c.commodity_name<<": Trying to get mutex on shared buffer\n";
	sem_wait(operation, mutex, semid);
	
	getTime();
	cout<<c.commodity_name<<": Placing "<< c.price<<" on shared buffer\n";
	
	strcpy(buffer[*index].commodity_name, c.commodity_name);
	buffer[*index].price = c.price;
	
	*index = ((*index)+1)%n;
	
	//getTime();
	//cerr<<": "<<buffer[*index].commodity_name<<" Added successfully"<<"\n";
	
	//getTime();
	//cerr<<": Signaling mutex\n";
	sem_signal(operation, mutex, semid);
	
	//getTime();
	//cerr<<": Signaling available\n";
	sem_signal(operation, available, semid);
	
}

int main(int argc, char *argv[]){

	char *commodity_name = argv[1];
	double commodity_mean = std::stod (argv[2]);
	double commodity_stanDev = std::stod (argv[3]);
	int sleep_interval = std::stoi (argv[4]);
	int n = std::stoi (argv[5]);
	struct commodity c;
	
	strcpy(c.commodity_name, commodity_name);
	c.commodity_mean = commodity_mean;
	c.commodity_stanDev = commodity_stanDev;
	c.sleep_interval = sleep_interval;
	
	while(1)
	{
		getTime();
		cerr<<c.commodity_name<<": Generating a new value ";
		c.price = getNormalDist(c);
		
		produce(c, n);
		
		getTime();
		cerr<<c.commodity_name<<": Sleeping " << c.sleep_interval <<" ms\n";
		//cerr<<"+-------------------------------------+\n\n";
		
		sleep(c.sleep_interval/1000);
	}
	
	return 0;
}









