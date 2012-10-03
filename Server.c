#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <pthread.h>        /* for POSIX threads */
#include <sys/stat.h> 
#include <sys/types.h>
#include <dirent.h>

void ErrorHandler(char *errorMessage);  /* Error handling function */
void HandleTCPClient(int clntSocket);   /* TCP client handling function */
int CreateTCPServerSocket(unsigned short port); /* Create TCP server socket */
int AcceptTCPConnection(int servSock);  /* Accept TCP connection request */

char enquire[]="3";

char fpath[100];
char rep_name[100];
void *ThreadMain(void *arg);            /* Main program of a thread */

#define MAXPENDING 5    /* Maximum outstanding connection requests */

#define RCVBUFSIZE 32   /* Size of receive buffer */

int clients[20];
int cl_index=0;

char findex[20][50];
int i=0;

/* Structure of arguments to pass to client thread */
struct ThreadArgs
{
    int clntSock;                      /* Socket descriptor for client */
};

int main(int argc, char *argv[])
{
    DIR *dir;
    char echoBuffer[1024];
    int k=0;
    struct dirent *ent;
    int servSock;                    /* Socket descriptor for server */
    int clntSock;                    /* Socket descriptor for client */
    unsigned short echoServPort;     /* Server port */
    pthread_t threadID;              /* Thread ID from pthread_create() */
    struct ThreadArgs *threadArgs;   /* Pointer to argument structure for thread */
    
    if (argc != 3)     /* Test for correct number of arguments */
    {
        fprintf(stderr,"Usage:  %s <SERVER PORT> <REPOSITORY NAME>\n", argv[0]);
        exit(1);
    }
    
    
    echoServPort = atoi(argv[1]);  /* First arg:  local port */
    
    servSock = CreateTCPServerSocket(echoServPort);
    
    //////file indexing
    strcat(argv[2],"//");
    strcpy(rep_name,argv[2]);
    strcpy(fpath,argv[2]);
    dir = opendir (argv[2]);
    if (dir != NULL) {
        
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
          //  printf ("%s\n", ent->d_name);
            int len=strlen(ent->d_name);
            if(ent->d_name[len-1]=='t' && ent->d_name[len-2]=='x' && ent->d_name[len-1]=='t')
                strcpy(findex[i++],ent->d_name);
        }
        printf("\n");
      //  for(k=0;k<i;k++)
      //      printf ("%s\n", findex[k]);
        
        closedir (dir);
    } else {
        /* could not open directory */
        perror ("");
        return 0;
    }

    //////end of indexing
    
    for(k=0;k<9;k++)
        printf ("%s\n", findex[k]);
    /////setting file values to null
    int ast=0;
    for(ast=0;ast<9;ast++)
    {strcpy(fpath,rep_name);
        strcat(fpath,findex[ast]);  
        FILE *fp;
        printf("Resetting : %s\n",fpath);
        fp = fopen(fpath,"w"); /* open for writing */
        fprintf(fp,"\n%s", fpath);
        fclose(fp); /* close the file before ending program */
        strcpy(fpath,rep_name);
    }
    
    FILE *fp;
    char *config="CONFIG";
    printf("Resetting : cfg.txt\n");
    fp = fopen("cfg.txt","w"); /* open for writing */
    fprintf(fp,"%s", config);
    fclose(fp);
    
    ////end of set-null process
    
    for (;;) /* run forever */
    {
        clntSock = AcceptTCPConnection(servSock);
        clients[cl_index++]=clntSock;
       
        /* Create separate memory for client argument */
        if ((threadArgs = (struct ThreadArgs *) malloc(sizeof(struct ThreadArgs))) 
            == NULL)
            ErrorHandler("malloc() failed");
        threadArgs -> clntSock = clntSock;
        
        /* Create client thread */
        if (pthread_create(&threadID, NULL, ThreadMain, (void *) threadArgs) != 0)
            ErrorHandler("pthread_create() failed");
        //printf("with thread %ld\n", (long int) threadID);
    }
    /* NOT REACHED */
} // main

void *ThreadMain(void *threadArgs)
{
    int clntSock;                   /* Socket descriptor for client connection */
    
    /* Guarantees that thread resources are deallocated upon return */
    pthread_detach(pthread_self()); 
    
    /* Extract socket file descriptor from argument */
    clntSock = ((struct ThreadArgs *) threadArgs) -> clntSock;
    free(threadArgs);              /* Deallocate memory for argument */
    
    HandleTCPClient(clntSock);
    
    return (NULL);
}

void ErrorHandler(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}


int AcceptTCPConnection(int servSock)
{
    int clntSock;                    /* Socket descriptor for client */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned int clntLen;            /* Length of client address data structure */
    
    /* Set the size of the in-out parameter */
    clntLen = sizeof(echoClntAddr);
    
    /* Wait for a client to connect */
    if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr, 
                           &clntLen)) < 0)
        ErrorHandler("accept() failed");
    
    /* clntSock is connected to a client! */
    
    printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));
    
    return clntSock;
}

int CreateTCPServerSocket(unsigned short port)
{
    int sock;                        /* socket to create */
    struct sockaddr_in echoServAddr; /* Local address */
    
    /* Create socket for incoming connections */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        ErrorHandler("socket() failed");
    
    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(port);              /* Local port */
    
    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        ErrorHandler("bind() failed");
    
    /* Mark the socket so it will listen for incoming connections */
    if (listen(sock, MAXPENDING) < 0)
        ErrorHandler("listen() failed");
    
    return sock;
}

void HandleTCPClient(int clntSocket)
{
    char echoBuffer[RCVBUFSIZE];        /* Buffer for echo string */
    int recvMsgSize; 
    
    
    char buff[100];
    for(;;)
    {
    /* Receive message from client */
        strcpy(echoBuffer,"");
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
        ErrorHandler("recv() failed");

        echoBuffer[recvMsgSize] = '\0';  /* Terminate the string! */
        
        
        if(echoBuffer[0]==enquire[0])
        {
            printf("ENQUIRE REQUEST RECEIVED\nSending file list to the client...\n");
            if (send(clntSocket, findex, sizeof(findex), 0) != sizeof(findex))
                ErrorHandler("send() failed");
        }
        
        if(echoBuffer[0]=='1')
        {
            char ebuf[100];
            strcpy(ebuf,"");
            strcpy(fpath,rep_name);
            printf("READ REQUEST RECEIVED\n");
        
            if ((recvMsgSize = recv(clntSocket, ebuf, sizeof(ebuf), 0)) < 0)
                ErrorHandler("recv() failed");
            printf("Got file name\n");
            ebuf[recvMsgSize] = '\0';  /* Terminate the string! */
            printf("File name : %s\n", ebuf);
            
            strcat(fpath,ebuf);
            printf("File path : %s\n", fpath);
            FILE *fopen(), *fpr;
            fpr = fopen(fpath,"r");
            strcpy(buff,"");
            while(fgets(buff, 100, fpr));
            printf("Line read : %s\n",buff);
            fclose(fpr);
            
            if (send(clntSocket,buff, sizeof(buff), 0) != sizeof(buff))
                ErrorHandler("send() failed");
            strcpy(fpath,rep_name);
            printf("Line read(recheck) : %s\n",buff);
        }
        
        if(echoBuffer[0]=='2')
        {
            printf("WRITE REQUEST RECEIVED\n");
            strcpy(fpath,rep_name);
            
            char ebuff[100];
            strcpy(echoBuffer,"");
            if ((recvMsgSize = recv(clntSocket, ebuff, sizeof(ebuff), 0)) < 0)
                ErrorHandler("recv() failed");
            
            echoBuffer[recvMsgSize] = '\0';  /* Terminate the string! */
             printf("File name : %s\n", ebuff);
            strcat(fpath,ebuff);
            printf("File path : %s\n", fpath);
           
            char writereq[100];
            strcpy(writereq,"Write_Complete.");
            
            strcpy(ebuff,"");
            if ((recvMsgSize = recv(clntSocket, ebuff, sizeof(ebuff), 0)) < 0)
                ErrorHandler("recv() failed");
            
             printf("Value : %s\n", ebuff);
            
          
            FILE *fp;
            int index;
            fp = fopen(fpath,"a"); /* open for writing */
            fprintf(fp,"\n%s", ebuff);
            fclose(fp); /* close the file before ending program */
            
            printf("Sending WRITE STATUS\n");            
            if (send(clntSocket, writereq, sizeof(writereq), 0) != sizeof(writereq))
                ErrorHandler("send() failed");
            
             strcpy(fpath,rep_name);
        }
        
        if(echoBuffer[0]=='4')
        {
            printf("\nQUIT REQUEST RECEIVED\nClosing all Sockets and exiting.\n");
            int i;
            for(i=0;i<clntSocket;i++)
            close(clients[i]);
            exit(0);
        
        }
    

        /* Echo message back to client */
        if (send(clntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
            ErrorHandler("send() failed");
        
    }
    close(clntSocket);    /* Close client socket */
}