# dining_philosophers_using_sockets

#### Problem Statement

a. Create five child processes for philosophers, and five child processes for each of the chopsticks.

b. Initiate a pair of sockets for each connection to a server.

c. Avoid possible deadlock.

d. Close and unlink the sockets.

Description: Use the fork( ) system call to create a child process. The parent should do nothing but create ten child processes. 
The requests for granting and releasing chopsticks should be done by each child process representing a philosopher through communication 
with the two processes representing the philosopher’s left and right hand chopsticks.TCP/IP should be used for communication between philosophers 
and chopstick processes. The datagram type of sockets should be employed between the parent and child processes including both the philosopher and 
chopstick processes. Use write( ) and read() system calls with the connection-oriented communication. A solution that prevents deadlock or starvation
should be implemented.  For example the RightLeftDP algorithm (Lynch,1996) is a suitable algorithm. Five child processes are to be created for the five
philosophers. The philosophers change their states among “thinking”, “waiting”, and “eating” until all philosophers fulfil their eating
requirements(60 seconds total eating time). Print the status of each philosopher whenever there is a change in the status.

#### Solution

The given problem is solved using socket programming in C language.
