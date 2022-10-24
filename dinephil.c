#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define PORT 8000

struct recv {
	int num;
	int client_fd;
	int* code;
	int* busy;
};

void* serverRecv(void* recvData) {
	struct recv data = *((struct recv*) recvData);
	
	while(1) {
		if(recv(data.client_fd, data.code, sizeof(int), 0) < 0) {
			printf("Chopstick %d lost\n", data.num + 1);
			exit(1);
		}
		
		if(*(data.code) == 0) {
			// Chopstick sends status to philosopher
			
			if(send(data.client_fd, data.busy, sizeof(int), 0) < 0) {
				printf("Chopstick %d lost\n", data.num + 1);
				exit(1);
			}
		} else if (*(data.code) == 1) {
			if(*(data.busy) != 1) {
				// Philosopher makes chopstick busy
				
				*(data.busy) = 1;
				*(data.code) = 1;
			} else {
				// Chopstick already busy
				
				*(data.code) = 3;
			}
			
			if(send(data.client_fd, data.code, sizeof(int), 0) < 0) {
				printf("Chopstick %d lost\n", data.num + 1);
				exit(1);
			}
		} else if (*(data.code) == 2) {
			// Philosopher makes chopstick free
			
			*(data.busy) = 0;
		}
	}
}

void server(int num) {
	int server_fd, client_fd1, client_fd2;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	int code, busy = 0;
	
	if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Chopstick %d lost\n", num + 1);
		exit(1);
	}
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT + num);
	
	if(bind(server_fd, (struct sockaddr*) &address, addrlen) < 0) {
		printf("Chopstick %d lost\n", num + 1);
		exit(1);
	}
	
	if(listen(server_fd, 2) < 0) {
		printf("Chopstick %d lost\n", num + 1);
		exit(1);
	}
	
	if((client_fd1 = accept(server_fd, (struct sockaddr*) &address, (socklen_t*) &addrlen)) < 0) {
		printf("Chopstick %d lost\n", num + 1);
		exit(1);
	}
	
	if((client_fd2 = accept(server_fd, (struct sockaddr*) &address, (socklen_t*) &addrlen)) < 0) {
		printf("Chopstick %d lost\n", num + 1);
		exit(1);
	}
	
	// Two parallel threads to receive request from two philosophers
	
	pthread_t tid1, tid2;
	struct recv data1, data2;
	
	data1.num = num;
	data2.num = num;
	
	data1.client_fd = client_fd1;
	data2.client_fd = client_fd2;
	
	data1.code = &code;
	data2.code = &code;
	
	data1.busy = &busy;
	data2.busy = &busy;
	
	pthread_create(&tid1, NULL, serverRecv, &data1);
	
	pthread_create(&tid2, NULL, serverRecv, &data2);
	
    	pthread_join(tid1, NULL);
    	
    	pthread_join(tid2, NULL);
    	
	close(server_fd);
	close(client_fd1);
	close(client_fd2);
}

void client(int num) {
	int client_fd1, client_fd2, code;
	struct sockaddr_in serv_addr1, serv_addr2;
	
	if((client_fd1 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Philosopher %d left\n", num + 1);
		exit(1);
	}
	
	if((client_fd2 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Philosopher %d left\n", num + 1);
		exit(1);
	}
	
	serv_addr1.sin_family = AF_INET;
	serv_addr1.sin_addr.s_addr = INADDR_ANY;
	serv_addr1.sin_port = htons(PORT + num);
	
	serv_addr2.sin_family = AF_INET;
	serv_addr2.sin_addr.s_addr = INADDR_ANY;
	serv_addr2.sin_port = htons(PORT + (num + 1) % 5);
	
	if(connect(client_fd1, (struct sockaddr*) &serv_addr1, sizeof(serv_addr1)) < 0) {
		printf("Philosopher %d left\n", num + 1);
		exit(1);
	}
	
	if(connect(client_fd2, (struct sockaddr*) &serv_addr2, sizeof(serv_addr2)) < 0) {
		printf("Philosopher %d left\n", num + 1);
		exit(1);
	}
	
	while(1) {
		sleep(1);
		
		code = 0;
		
		// Philosopher requests status of left chopstick
		
		printf("Philosopher %d is hungry\n", num + 1);
		
		if(send(client_fd1, &code, sizeof(int), 0) < 0) {
			printf("Philosopher %d left\n", num + 1);
			exit(1);
		} else {
			int busy;
			
			if(recv(client_fd1, &busy, sizeof(int), 0) < 0) {
				printf("Philosopher %d left\n", num + 1);
				exit(1);
			} else {
				if(busy) {
					//Chopstick busy
					
					continue;
				} else {
					code = 0;
					
					// Philosopher requests status of right chopstick
					
					if(send(client_fd2, &code, sizeof(int), 0) < 0) {
						printf("Philosopher %d left\n", num + 1);
						exit(1);
					} else {
												
						if(recv(client_fd2, &busy, sizeof(int), 0) < 0) {
							printf("Philosopher %d left\n", num + 1);
							exit(1);
						} else {
							if(busy) {
								// Chopstick busy
								
								continue;
							} else {
								code = 1;
								
								// Philosopher tries to make left chopstick busy
								
								if(send(client_fd1, &code, sizeof(int), 0) < 0) {
									printf("Philosopher %d left\n", num + 1);
									exit(1);
								} else {
									if(recv(client_fd1, &code, sizeof(int), 0) < 0) {
										printf("Philosopher %d left\n", num + 1);
										exit(1);
									} else {
										if(code == 3) // Chopstick already busy
											continue;
									}
								}
								
								// Philosopher tries to make right chopstick busy
								
								if(send(client_fd2, &code, sizeof(int), 0) < 0) {
									exit(1);
									printf("Philosopher %d left\n", num + 1);
								} else {
									if(recv(client_fd2, &code, sizeof(int), 0) < 0) {
										printf("Philosopher %d left\n", num + 1);
										exit(1);		
									} else {
										if(code == 3) {
											// Chopstick already busy
											
											code = 2;
											
											// Philosopher makes left chopstick free
											
											if(send(client_fd1, &code, sizeof(int), 0) < 0) {
												printf("Philosopher %d left\n", num + 1);
												exit(1);
											} 
											
											continue;
										}
									}
								}
								
								sleep(1);
								
								printf("Philosopher %d takes chopsticks %d and %d up and is eating\n", num + 1, num + 1, ((num + 1) % 5) + 1);
								
								sleep(1);
								
								printf("Philosopher %d puts chopsticks %d and %d down and is thinking\n", num + 1, num + 1, ((num + 1) % 5) + 1);
								
								code = 2;
								
								// Philosopher frees both chopsticks
								
								if(send(client_fd1, &code, sizeof(int), 0) < 0) {
									printf("Philosopher %d left\n", num + 1);
									exit(1);
								}
								
								if(send(client_fd2, &code, sizeof(int), 0) < 0) {
									printf("Philosopher %d left\n", num + 1);
									exit(1);
								}
								
								// Philosophers who ate wait longer for priority service
								
								sleep(2);
							}
						}
					}
				}
			}
		}
	}
	
	close(client_fd1);
	close(client_fd2);
}

void main() {
	printf("There are 5 philosophers and 5 chopsticks\n");	
	
	for (int i = 0; i < 5; i++) {
		printf("Philosopher %d is thinking\n", i + 1);
		
		if(fork() != 0) {
			server(i); // 5 Chopsticks
			break;
		}
	}
	
	for (int i = 0; i < 5; i++) {
		if(fork() != 0) {
			client(i); // 5 Philosophers
			break;
		}
	}
}
