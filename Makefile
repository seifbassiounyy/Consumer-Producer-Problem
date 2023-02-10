#Use GNU compiler
cc = gcc -g
CC = g++ -g

all: producer consumer

producer:
	$(CC) producer.cpp -o producer
	
consumer:
	$(CC) consumer.cpp -o consumer
	
clean:
	rm -f producer consumer
