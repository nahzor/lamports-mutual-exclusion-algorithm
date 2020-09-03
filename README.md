# Lamport's Mutual Exclusion Algorithm

<b>Files in the repo:</b>
1. client1.c  -  Client code
2. Server.c - Server code
3. repository1, repository2, repository3 - the file repositories for 3 different servers.
4. cfg.txt - file used for client connectivity

The Server.c file should be run first.

<b>Command line arguments to run 'Server':</b>
./a.out &lt;SERVER-PORT-NUMBER&gt; &lt;REPOSITORY-NAME&gt;

REPOSITORY NAME can be:
repository1
repository2
repository3 

<b>Command line arguments to run 'client1':</b>

./a.out &lt;UNIQUE NAME&gt; &lt;IP ADDRESS OF SERVER 1&gt; &lt;PORT NUMBER OF SERVER 1&gt; &lt;IP ADDRESS OF SERVER 2&gt; &lt;PORT NUMBER OF SERVER 2&gt; &lt;IP ADDRESS OF SERVER 3&gt; &lt;PORT NUMBER OF SERVER 3&gt;

<b>Note:</b>

Once the code starts running, it will ask for a Port number to start a TCP server in the client1.c program.

In the end, when the execution stops, press ENTER in one of the client windows to close sockets and end all programs.

If  you are using a different repository, please use a repository with 9 txt files as I have hard-coded the value for certain iterations and was not able to change that in time.

I have given 20 random READ/WRITE for random files through random servers using the rand() function.

All the repository files and the cfg.txt files will be cleared or data each time the servers start.

<b>Reference:</b>
http://en.wikipedia.org/wiki/Lamport%27s_Distributed_Mutual_Exclusion_Algorithm
