AOS Project 1

Files in the zip file:
1. client1.c  -  Client code
2. Server.c - Server code
3. repository1, repository2, repository3 - the file repositories for 3 different servers.
4. cfg.txt - file used for client connectivity

The Server.c file should be run first.

Command line arguments to run 'Server':
./a.out <SERVER PORT NUMBER> <REPOSITORY NAME>
REPOSITORY NAME can be:
repository1
repository2
repository3 

Command line arguments to run 'client1':

./a.out <UNIQUE NAME> <IP ADDRESS OF SERVER 1><PORT NUMBER OF SERVER 1><IP ADDRESS OF SERVER 2><PORT NUMBER OF SERVER 2><IP ADDRESS OF SERVER 3><PORT NUMBER OF SERVER 3>

Once the code starts running, it will ask for a Port number to start a TCP server in the client1.c program.

In the end, when the execution stops, press ENTER in one of the client windows to close sockets and end all programs.

If  you are using a different repository, please use a repository with 9 txt files as I have hard-coded the value for certain iterations and was not able to change that in time.

I have given 20 random READ/WRITE for random files through random servers using the rand() function.

All the repository files and the cfg.txt files will be cleared or data each time the servers start.

Thank you.
