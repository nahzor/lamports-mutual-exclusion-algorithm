//
//  client1.c
//

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#define NUM_THREADS     5
#define MAXPENDING 5
#define RCVBUFSIZE 32

int qsent=0;
int no_of_replies0=0;
int no_of_replies1=0;
int no_of_replies2=0;
int no_of_replies3=0;
int timestamp=0;
char my_ip[50];
char str[80];
int clients[4];
unsigned short serv1Port, serv2Port, serv3Port;     /* Echo server port */
char *serv1IP, *serv2IP, *serv3IP,uid[50];
int eq_check=0;
int peer_num=0;
char PQ[500][7][100];
char PQ_rcv[7][100];
int pq_read_point=0;
int pq_write_point=0;
int pqts=0,pqip=1,pqport=2,pqfname=3,pqmsgtype=4,pqreqtype=5,pqservname=6;
int pq_mutex=0;
int sock1 ,sock2 ,sock3,sock[3];
char ips_in_front[10][50];
char ports_in_front[10][50];

int sock_saver[40];
int sock_saver_index=0;


int total_reqs=0;
//int fc=0;
int my_pos=0;

int client_count=0;
void *ThreadMain(void *arg);            /* Main program of a thread */

/* Structure of arguments to pass to client thread */
struct ThreadArgs
{
    int clntSock;                      /* Socket descriptor for client */
};


void ErrorHandler(char *errorMessage);  /* Error handling function */
void Client_Request_Receiver(int clntSocket);   /* TCP client handling function */
int CreateTCPServerSocket(unsigned short port); /* Create TCP server socket */
int AcceptTCPConnection(int servSock);  /* Accept TCP connection request */

void *TCPserver(void *threadid)
{
    long tid;
    tid = (long)threadid;
    printf("Starting Server..\n");
    int servSock;                    /* Socket descriptor for server */
    int clntSock;                    /* Socket descriptor for client */
    unsigned short echoServPort;     /* Server port */
    pthread_t threadID;              /* Thread ID from pthread_create() */
    struct ThreadArgs *threadArgs;   /* Pointer to argument structure for thread */
    
    
    echoServPort = atoi(str);  /* First arg:  local port */
    
    servSock = CreateTCPServerSocket(echoServPort);
    
    for (;;) /* run forever */
    {
        clntSock = AcceptTCPConnection(servSock);
        
        sock_saver[sock_saver_index++]=clntSock;
        
        /* Create separate memory for client argument */
        if ((threadArgs = (struct ThreadArgs *) malloc(sizeof(struct ThreadArgs))) == NULL)
            ErrorHandler("malloc() failed");
        threadArgs -> clntSock = clntSock;
        
        /* Create client thread */
        if (pthread_create(&threadID, NULL, ThreadMain, (void *) threadArgs) != 0)
            ErrorHandler("pthread_create() failed");
       // printf("with thread %ld\n", (long int) threadID);
    }
    /* NOT REACHED */
    pthread_exit(NULL);
}


void *Client_Controller(void *threadid)
{
    long tid;
    tid = (long)threadid;
    int bytes_recieved=0;/* Socket descriptor */
    int servnum=0, filenum=0;
    char recv_data[1024];
    struct sockaddr_in ServAddr1, ServAddr2, ServAddr3; /* Echo server address */
    char request[100];  /* String to send to echo server */
    char findex[20][50];
    int fileindex=0;
    unsigned int StringLen;      /* Length of string to echo */
    int bytesRcvd, totalBytesRcvd;
    

    
    if ((sock1 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        ErrorHandler("socket() failed");
    
    if ((sock2 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        ErrorHandler("socket() failed");
    
    if ((sock3 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        ErrorHandler("socket() failed");
    
    memset(&ServAddr1, 0, sizeof(ServAddr1));     
    ServAddr1.sin_family      = AF_INET;            
    ServAddr1.sin_addr.s_addr = inet_addr(serv1IP);   /* Server IP address */
    ServAddr1.sin_port        = htons(serv1Port); /* Server port */
    
    memset(&ServAddr2, 0, sizeof(ServAddr2));     
    ServAddr2.sin_family      = AF_INET;            
    ServAddr2.sin_addr.s_addr = inet_addr(serv2IP);   /* Server IP address */
    ServAddr2.sin_port        = htons(serv2Port); /* Server port */
    
    memset(&ServAddr3, 0, sizeof(ServAddr3));     
    ServAddr3.sin_family      = AF_INET;            
    ServAddr3.sin_addr.s_addr = inet_addr(serv3IP);   /* Server IP address */
    ServAddr3.sin_port        = htons(serv3Port); /* Server port */
    
    /* Establish the connection to the echo server */
    if (connect(sock1, (struct sockaddr *) &ServAddr1, sizeof(ServAddr1)) < 0)
        ErrorHandler("connect() failed");
    
    if (connect(sock2, (struct sockaddr *) &ServAddr2, sizeof(ServAddr2)) < 0)
        ErrorHandler("connect() failed");
    
    if (connect(sock3, (struct sockaddr *) &ServAddr3, sizeof(ServAddr3)) < 0)
        ErrorHandler("connect() failed");
    
    sock[1]=sock1;
    sock[2]=sock2;
    sock[3]=sock3;
    
    char req;
    
    while(1)
    {
        strcpy(request,"");
        
        while(sock_saver_index<4);    // waits for other clients to join..slick!
        printf("No. of peer clients : %d\n",sock_saver_index);
        total_reqs++;
        
        printf("\nAction to perform : \n1.READ\n2.WRITE\n3.ENQUIRE\n4.EXIT\nACTION>>");
              //  scanf(" %s",request);
        sleep(2);
        if(total_reqs<20)
        {
            if((total_reqs%2)==0)
                strcpy(request,"1");
            else if((total_reqs%2)==1)
                strcpy(request,"2");
        }
        else
        {
            strcpy(request,"4");
            printf("Press ENTER to exit once all computations are complete.\n");
            getchar();getchar();
            
        }
        
        
        
        
        if(request[0]!='4' && request[0]!='3' && eq_check==0)
        {
            printf("Error:File list not available.\nSending enquire request to obtain file list.\n");
            strcpy(request,"3");
        }
        
        
        StringLen = sizeof(request);          /* Determine input length */
        
        if(request[0]=='1' || request[0]=='3' )
        {
        printf("\nServer to send the request to?(1/2/3) : ");
        //scanf(" %d",&servnum);
            if((total_reqs%3)==0)
                servnum=1;
            else if((total_reqs%3)==1)
                servnum=2;
            else if((total_reqs%3)==2)
                servnum=3;
                 
        }
           
        if(request[0]=='1' || request[0]=='2')
        {
            
            while(pq_mutex==1);
            pq_mutex=1;
            sprintf(PQ[pq_write_point][pqts],"%d",timestamp);
            strcpy(PQ[pq_write_point][pqreqtype],request);
            strcpy(PQ[pq_write_point][pqip],my_ip);
            strcpy(PQ[pq_write_point][pqport],str);
            strcpy(PQ[pq_write_point][pqmsgtype],"req");
           
            if(request[0]=='2')   
                strcpy(PQ[pq_write_point][pqservname],"all");
            else
            sprintf(PQ[pq_write_point][pqservname],"%d",servnum);
        
            /////
            req=request[0];
            if(eq_check!=0)
            {
                int i;
                for(i=0;i<9;i++)
                {
                    printf("%d : %s\n",i,findex[i]);
                }
                
                printf("File name on which the R/W has to be performed on : ");
                strcpy(request,"");
                //scanf(" %d",&filenum);
                
                //if(fc>8)fc=0;
                
                filenum=rand()%4;
                
                strcpy(request,findex[filenum]);
                strcpy(PQ[pq_write_point][pqfname],findex[filenum]);
                
                int j;
                for(j=0;j<7;j++)
                {
                    printf("PQ @ write pt : %s\n",PQ[pq_write_point][j]);
                }
                /*
                 no_of_replies0=0;
                 no_of_replies1=0;
                 no_of_replies2=0;
                 no_of_replies3=0;
                 */
                for(i=0;i<sock_saver_index;i++)
                {
                    timestamp++;
                    if (send(sock_saver[i], PQ[pq_write_point], sizeof(PQ[pq_write_point]), 0) != sizeof(PQ[pq_write_point]))
                        ErrorHandler("send()1 failed");
                }
                pq_write_point++;
                StringLen = sizeof(request);
                
                
            } 
            /////
        
        pq_mutex=0;
    }

        
      
        
        printf("Sending String : %s\n",request);
        
        
         //request_sender
        if(request[0]=='4' )
        {
            printf("Sending EXIT messge to Servers\n");
            timestamp++;
            if (send(sock[1], request, StringLen, 0) != StringLen)
                ErrorHandler("send2() sent a different number of bytes than expected");
           
            timestamp++;
            if (send(sock[2], request, StringLen, 0) != StringLen)
                ErrorHandler("send3() sent a different number of bytes than expected");
            
            timestamp++;
            if (send(sock[3], request, StringLen, 0) != StringLen)
                ErrorHandler("send1() sent a different number of bytes than expected");    
      
        }
        else if(request[0]=='3' )
        {
           timestamp++;
            if (send(sock[servnum], request, StringLen, 0) != StringLen)
                ErrorHandler("send() sent a different number of bytes than expected");
        }
        
        
        if(request[0]=='3')
        {
            eq_check=1;  
           
            
            printf("Received: \n");                /* Setup to print the echoed string */
            
            timestamp++;
            if ((bytesRcvd = recv(sock[servnum], findex, sizeof(findex), 0)) <= 0)
                ErrorHandler("recv()2 failed or connection closed prematurely");
            totalBytesRcvd += bytesRcvd;   /* Keep tally of total bytes */
            int k=0;
            for(k=0;k<9;k++)
                printf ("%s\n", findex[k]);
        
            
        }
                       
        if(request[0]=='4')
                       {  
                           
                        
                           close(sock1);
                           close(sock2);
                           close(sock3);
                           
                           printf("Sending EXIT messge to Peers\n");
                           int i; 
                           for(i=0;i<sock_saver_index;i++)
                           {
                               timestamp++;  
                               char end_send[7][100];
                               strcpy(end_send[6],request);
                               if (send(sock_saver[i], end_send, sizeof(end_send), 0) != sizeof(end_send))
                                   ErrorHandler("send() sent a different number of bytes than expected");
                               //close(sock_saver[i]);
                           }
                           
                           printf("\nTerminating Execution\n");    /* Print a final linefeed */
                           exit(0);
                       }
                       
                       
                       
                       }//end of while

    pthread_exit(NULL);
                       }



//the clienttcp
void *Peer_Handler(void *threadid)
{
    long tid;
    tid = (long)threadid;
        int sock;                        /* Socket descriptor */
    struct sockaddr_in echoServAddr; /* Echo server address */
    unsigned short echoServPort;     /* Echo server port */
    char *servIP;                    /* Server IP address (dotted quad) */
    char *echoString;                /* String to send to echo server */
    char echoBuffer[RCVBUFSIZE];     /* Buffer for echo string */
    unsigned int echoStringLen;      /* Length of string to echo */
    int bytesRcvd, totalBytesRcvd;   /* Bytes read in single recv() */
    int clntSocket;
    

    if(peer_num<my_pos){
    servIP = ips_in_front[peer_num];             /* First arg: server IP address (dotted quad) */
  
    echoServPort = atoi(ports_in_front[peer_num++]); /* Use given port, if any */
                          }
    
    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        ErrorHandler("socket() failed");
    
    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    echoServAddr.sin_family      = AF_INET;             /* Internet address family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
    echoServAddr.sin_port        = htons(echoServPort); /* Server port */
    
    /* Establish the connection to the echo server */
    if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        ErrorHandler("connect() failed");
    
    sock_saver[sock_saver_index++]=sock;
    
    echoString = "hello"; 
    echoStringLen = sizeof(echoString);          /* Determine input length */
    
    /* Send the string to the server */
   // if (send(sock, echoString, echoStringLen, 0) != echoStringLen)
   //     ErrorHandler("send() sent a different number of bytes than expected");
    
    /* Receive the same string back from the server */
    totalBytesRcvd = 0;
    printf("Received: ");                /* Setup to print the echoed string */
    while (1)
    {
        /* Receive up to the buffer size (minus 1 to leave space for
         a null terminator) bytes from the sender */
        int i;
        for(i=0;i<6;i++)
            strcpy(PQ_rcv[i],"");
        int recvMsgSize;
        if ((recvMsgSize = recv(sock, PQ_rcv, sizeof(PQ_rcv), 0)) < 0)   //tupdate
            ErrorHandler("recv()3 failed");
        
        if(strncmp(PQ_rcv[6],"4",1)==0)
        {
            printf("Closing Sockets and terminating Execution\n");
            close(sock1);
            close(sock2);
            close(sock3);
            int i;
            for(i=0;i<sock_saver_index;i++)
            {
                close(sock_saver[i]);
            }
            exit(0);
        }
        
        if(timestamp>atoi(PQ_rcv[pqts]))
        {
            timestamp++;
        }
        else
        {
            timestamp=atoi(PQ_rcv[pqts]);
            timestamp++;
        }
        
        for(i=0;i<7;i++)
            printf("Received message : %s\n",PQ_rcv[i]);
        
        if(PQ_rcv[pqmsgtype][2]=='q')
        {
            
        printf("PH:ITS A REQ MSG!\n");
            
            while(pq_mutex==1);
            pq_mutex=1;
            printf("PH-q:Mutex obtained\n");
            int i;int index;
            
            if(atoi(PQ_rcv[pqts])>timestamp)
            {
             if(sock==sock_saver[0])
                no_of_replies0++;
             else if(sock==sock_saver[1])
                 no_of_replies1++;
             else if(sock==sock_saver[2])
                 no_of_replies2++;
             else if(sock==sock_saver[3])
                 no_of_replies3++;
                    
            
            }
            for(i=pq_read_point;i<=pq_write_point;i++)
            {
                index=i;   
                if(atoi(PQ_rcv[pqts])<atoi(PQ[i][pqts]))    
                {
                    index=i;break;  
                }
            }
            printf("Insert point : %d",index);
            char temp[7][100];
            char temp_send[7][100];
            
            for(i=0;i<7;i++)
            {
                strcpy(temp_send[i],PQ_rcv[i]);
            }
            
            int j;
            for(i=index;i<=pq_write_point;i++)
            {
                for(j=0;j<7;j++)
                {
                strcpy(temp[j],PQ[i][j]);
                }
                for(j=0;j<7;j++)
                {
                strcpy(PQ[i][j],PQ_rcv[j]);
                }
                for(j=0;j<7;j++)
                {
                strcpy(PQ_rcv[j],temp[j]);
                }
            }
            
            for(j=0;j<7;j++)
            {
                printf("Inserted : %s\n",PQ[index][j]);   
            }
            
            pq_write_point++;
            strcpy(temp_send[pqmsgtype],"rep");
            int recvMsgSize=sizeof(temp_send);
          timestamp++;
            if (send(sock, temp_send, recvMsgSize, 0) != recvMsgSize)
                ErrorHandler("send()2 failed");
            
            
            pq_mutex=0;
            printf("PH-q:Mutex released\n");
            
        }
        else if(PQ_rcv[pqmsgtype][2]=='p')
        {
            printf("PH:ITS A REP MSG!\n");
           
             if(atoi(PQ_rcv[pqts])>timestamp) 
             {
                 if(sock==sock_saver[0])
                     no_of_replies0++;
                 else if(sock==sock_saver[1])
                     no_of_replies1++;
                 else if(sock==sock_saver[2])
                     no_of_replies2++;
                 else if(sock==sock_saver[3])
                     no_of_replies3++;
                 
                 
             }
        }
        else if(PQ_rcv[pqmsgtype][2]=='l')
        {
            printf("PH:ITS A REL MSG!\n");
            while(pq_mutex==1);
            pq_mutex=1;
            printf("PH-l:Mutex obtained\n");
            int i;int index;
           
            
            
             if(atoi(PQ_rcv[pqts])>timestamp)
             {
                 if(sock==sock_saver[0])
                     no_of_replies0++;
                 else if(sock==sock_saver[1])
                     no_of_replies1++;
                 else if(sock==sock_saver[2])
                     no_of_replies2++;
                 else if(sock==sock_saver[3])
                     no_of_replies3++;
                 
                 
             }
            
            for(i=pq_read_point;i<pq_write_point;i++)
            {
                int len=strlen(PQ_rcv[pqip]);
                if(strncmp(PQ_rcv[pqip],PQ[i][pqip],len)==0)
                {
                    printf("PH:same ip\n");
                    if(strncmp(PQ_rcv[pqport],PQ[i][pqport],4)==0)
                    {  
                        printf("PH:same port\n");
                        int flen=strlen(PQ_rcv[pqfname]);
                        printf("File names::%c | %s\n",PQ_rcv[pqfname][4],PQ[i][pqfname]);
                        if(PQ_rcv[pqfname][4]==PQ[i][pqfname][4])
                                        {
                                            printf("PH:Got the file to rel\n");
                                        index=i;break;  
                                           
                                        }
                                        } 
                }
            }
     
              for(i=index;i<=pq_write_point;i++)
            {
                int j;
                for(j=0;j<7;j++)
                {
                strcpy(PQ[i][j],PQ[i+1][j]);
                } 
            }
            pq_write_point--;
        
            
            printf("pqwp: %d\n",pq_write_point);
            pq_mutex=0;
            printf("PH-l:Mutex released\n");
            
        }
        
        
    }
    
    printf("\n");    /* Print a final linefeed */
    
    close(sock);
    exit(0);
    pthread_exit(NULL);
}


void *Query_Sender(void *threadid)
{
 while(1)
 {
     char fname[50];qsent=0;
     while(no_of_replies0<0 && no_of_replies0<1 && no_of_replies0<2 && no_of_replies3<1); //rosh
     no_of_replies0=0;
      no_of_replies1=0;
      no_of_replies2=0;
      no_of_replies3=0;
     qsent=1;
     while(pq_write_point<1);
     while(pq_mutex==1);
     //printf(".");
     pq_mutex=1;int flag=1;
     char request[7][100];
     char trequest[7][100];
     strcpy(fname,"");
     int i;int index;
  
     for(i=pq_read_point;i<pq_write_point;i++)
     {
         
         int len=strlen(my_ip);
         if(strncmp(my_ip,PQ[i][pqip],len)==0)
         {
             
             if(strncmp(str,PQ[i][pqport],4)==0)
             {  
             
                     index=i;
                 strcpy(fname,PQ[i][pqfname]);
                 int j;flag=0;
                 for(j=pq_read_point;j<index;j++)
                 {
                     if(strcmp(fname,PQ[j][pqfname])==0)
                     {
                        
                         flag=1;
                         break;
                         
                     }
                 }
             }
         }//first if
                     if(flag==0)
                     {
                         printf("Sending request from PQ\n");
                         int j;
                         for(j=0;j<7;j++)
                         {
                         strcpy(request[j],PQ[index][j]);
                             strcpy(trequest[j],PQ[index][j]);
                         }
                         
                         printf("Printing request content:\n");
                         for(j=0;j<7;j++)
                         {
                             printf("%s\n",request[j]); // data goes missin?
                         }
                         
                         char recv_data[200];
                         char req[100];
                             
                         if(request[pqreqtype][0]=='2'){
                             
                             printf("Sending Write request to servers\n");
                             int StringLen=32;
                            timestamp++;
                             if (send(sock[1], request[pqreqtype], StringLen, 0) != StringLen)
                                 ErrorHandler("send2() sent a different number of bytes than expected");
                             timestamp++;
                             if (send(sock[2], request[pqreqtype], StringLen, 0) != StringLen)
                                 ErrorHandler("send3() sent a different number of bytes than expected");
                             timestamp++;
                             if (send(sock[3], request[pqreqtype], StringLen, 0) != StringLen)
                                 ErrorHandler("send1() sent a different number of bytes than expected");  
                             
                             strcpy(req,fname);
                             printf("Entering Write mode and sending file name :%s \n",req);
                             timestamp++;
                             if (send(sock[3], req, sizeof(req), 0) != sizeof(req))
                                 ErrorHandler("send() sent a different number of bytes than expected");
                             timestamp++; 
                             if (send(sock[1], req, sizeof(req), 0) != sizeof(req))
                                 ErrorHandler("send() sent a different number of bytes than expected");
                             timestamp++; 
                             if (send(sock[2], req, sizeof(req), 0) != sizeof(req))
                                 ErrorHandler("send() sent a different number of bytes than expected");
                             
                             strcpy(req,"< ");
                             
                             char temp[50];
                             //sprintf(temp,"%d",PQ[index][timestamp]);
                             strcpy(temp,PQ[index][pqts]);
                             
                             int bytes_recieved;
                             strcpy(recv_data,"");
                             strcat(req,uid);
                             strcat(req," : ");
                             strcat(req,temp);
                             strcat(req," >");
                             
                             printf("Request itoa : %s\n",req);
                             timestamp++;
                             if (send(sock[1], req, sizeof(req), 0) != sizeof(req))
                                 ErrorHandler("send() sent a different number of bytes than expected");
                             timestamp++; 
                             if (send(sock[2], req, sizeof(req), 0) != sizeof(req))
                                 ErrorHandler("send() sent a different number of bytes than expected");
                             timestamp++; 
                             if (send(sock[3], req, sizeof(req), 0) != sizeof(req))
                                 ErrorHandler("send() sent a different number of bytes than expected");
                             
                             int l=0;
                             for(l=1;l<4;l++){
                                 strcpy(req,"");
                                 timestamp++;
                                 bytes_recieved=recv(sock[l],req,sizeof(req),0);
                                 req[bytes_recieved] = '\0';
                                 printf("recv_data : %s\n",req);
                             }
                             
                             
                             
                         }
                         else if(request[pqreqtype][0]=='1')
                         {
                             printf("Sending Read request to server\n");
                             int StringLen=32;
                            timestamp++;
                             if (send(sock[atoi(PQ[index][pqservname])], request[pqreqtype], StringLen, 0) != StringLen)
                                 ErrorHandler("send() sent a different number of bytes than expected");
                             
                             
                             strcpy(req,fname);
                            
                             printf("Entering Read mode : %s\n",req);
                             timestamp++;
                             printf("Sending to : %d\n",atoi(PQ[index][pqservname]));
                             if (send(sock[atoi(PQ[index][pqservname])], req, sizeof(req), 0) != sizeof(req))
                                 ErrorHandler("send() sent a different number of bytes than expected");
                             strcpy(recv_data,"");
                             printf("File name SENT:%s\n",req);
                             timestamp++;
                             strcpy(req,"");
                             int bytes_recieved=recv(sock[atoi(PQ[index][pqservname])],req,sizeof(req),0);
                             recv_data[bytes_recieved] = '\0';
                             printf("########Value received : %s########\n\n",req);
                         }
                         
                         //while(pq_mutex==1);
                        // pq_mutex=1;
                         printf("Time to release the request\n");
                       
                         for(j=0;j<7;j++)
                         {
                             printf("Copying:REL: %s\n",request[j]);
                            // strcpy(request[j],PQ[index][j]);
                         }
                         
                         for(j=0;j<7;j++)
                         {
                             printf("trequest: %s\n",trequest[j]);
                             // strcpy(request[j],PQ[index][j]);
                         }
                         
                         for(i=index;i<=pq_write_point;i++)
                         {
                             
                             int j;
                             for(j=0;j<7;j++)
                             {
                             strcpy(PQ[i][j],PQ[i+1][j]);
                             }
                         }
                         
                         pq_write_point--;
                        
                         printf("Values in PQ : %d\n",pq_write_point);
                        
                         printf("Release request loaded\n");
                         
                         strcpy(request[pqmsgtype],"rel");
                         int i;
                         printf("Sending Release msg to all peers : %d\n",sock_saver_index);  
                         for(i=0;i<sock_saver_index;i++)
                         {
                           timestamp++;  
                         if (send(sock_saver[i], request, sizeof(request), 0) != sizeof(request))
                             ErrorHandler("send() sent a different number of bytes than expected");
                         }
                         

                     }//if flag==0
        
     }//for
 
pq_mutex=0;
     int tp;
     for(tp=0;tp<2000;tp++);
   flag=0;
 }//while
    
pthread_exit(NULL);
}//func

int main (int argc, char *argv[])
{
    pthread_t threads[NUM_THREADS];
    int rc;
    long t=0;
    
    if (argc!=8)    /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage: %s <UID> <Server1 IP> <Server1 Port> <Server2 IP> <Server2 Port> <Server3 IP> <Server3 Port>\n",
                argv[0]);
        exit(1);
    }
    
    strcpy(uid,argv[1]);
    
    serv1IP = argv[2];            
    serv1Port = atoi(argv[3]);
    serv2IP = argv[4];            
    serv2Port = atoi(argv[5]);
    serv3IP = argv[6];            
    serv3Port = atoi(argv[7]);
    
    ///my ip
    char Buf [ 200 ] ;
    struct hostent * Host = (struct hostent * ) malloc ( sizeof ( struct hostent ));
    gethostname ( Buf , 200 ) ;
    printf ( "%s\n", Buf ) ;
    Host = ( struct hostent * ) gethostbyname ( Buf ) ;
    strcpy(my_ip,inet_ntoa(*((struct in_addr *)Host->h_addr)));
    printf("My IP Address : %s\n", my_ip);
    int iplen=strlen(my_ip);
 
    ////my ip
    
    
   
    
    printf("Enter the TCP server port for client  : ");
    scanf("%s",str);
    
    /////writing to cfg

        FILE *fp;
        printf("Writing to cfg.txt\n");
        fp = fopen("cfg.txt","a"); /* open for writing */
        fprintf(fp,"\n%s", my_ip);
        fprintf(fp,"\n%s", str);
        fclose(fp); /* close the file before ending program */
        

    /////end of writing to cfg
    
    ///////getting ip/ports in front and my position
    char buff[200];
    char buff1[200];
    FILE *fopen(), *fpr;
    fpr = fopen("cfg.txt","r");
    strcpy(buff,"");
    int fcount=0;
   
    fgets(buff, 200, fpr);
    while(fgets(buff, 200, fpr))
    {
        strcpy(ips_in_front[fcount],buff);
        printf("Reading IP : %s\n",ips_in_front[fcount]);
        strcpy(buff,"");
        fgets(buff1, 200, fpr);
        strcpy(ports_in_front[fcount++],buff1);
         printf("Reading Port : %s\n",ports_in_front[fcount-1]);
         strcpy(buff1,"");
   
        if(strncmp(ips_in_front[fcount-1],my_ip,iplen)==0 && strncmp(ports_in_front[fcount-1],str,4)==0)
        {
            printf("MATCH FOUND!\n");
            my_pos=fcount-1;break;
        }
            
          }
    fclose(fpr);
    ///////end of:getting ip/ports in front and my position
    
    printf("CLIENT : %s\n",uid);
    
    printf("main:Starting a server\n");
    if(my_pos!=4)
    {
        
    rc = pthread_create(&threads[t], NULL, TCPserver, (void *)t);
    if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }
        else printf("Not Starting TCP server since this is the 5th process\n");
    t++;
    }
    
    printf("my_pos : %d\n",my_pos);
    
    int iter;
    for(iter=0;iter<my_pos;iter++)
    {
        printf("main:Starting a client:\n");
    rc = pthread_create(&threads[t], NULL, Peer_Handler, (void *)t);
    if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);}
    t++;
    }
    
    printf("main:starting client_controller\n");
    rc = pthread_create(&threads[t], NULL, Client_Controller, (void *)t);
    if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);}
    t++;
    
     printf("main:starting Query_Sender\n");
    rc = pthread_create(&threads[t], NULL, Query_Sender, (void *)t);
    if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);}
    
    /* Last thing that main() should do */
    pthread_exit(NULL);
}

void *ThreadMain(void *threadArgs)
{
   
    int clntSock;                   /* Socket descriptor for client connection */
    
    /* Guarantees that thread resources are deallocated upon return */
    pthread_detach(pthread_self()); 
    
    /* Extract socket file descriptor from argument */
    clntSock = ((struct ThreadArgs *) threadArgs) -> clntSock;
    free(threadArgs);              /* Deallocate memory for argument */
    
    Client_Request_Receiver(clntSock);
    
    return (NULL);
}

void ErrorHandler(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
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

//Belongs to tcpserver
void Client_Request_Receiver(int clntSocket)
{
    
    char echoBuffer[RCVBUFSIZE];        /* Buffer for echo string */
    int recvMsgSize; 
    
    clients[client_count++]=clntSocket;
    printf("No. of connected clients : %d\n",client_count);

    while (1)      /* zero indicates end of transmission */
    {
        /* Echo message back to client */
 //       if (send(clntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
 //           ErrorHandler("send() failed");
        
        int i;
        for(i=0;i<6;i++)
            strcpy(PQ_rcv[i],"");
        
        if ((recvMsgSize = recv(clntSocket, PQ_rcv, sizeof(PQ_rcv), 0)) < 0) //tupdate
            ErrorHandler("recv()1 failed");
        
       
        if(strncmp(PQ_rcv[6],"4",1)==0)
        {
            printf("Closing Sockets and terminating Execution\n");
            close(sock1);
            close(sock2);
            close(sock3);
            int i;
            for(i=0;i<sock_saver_index;i++)
            {
                close(sock_saver[i]);
            }
            exit(0);
            
        }
        
        if(timestamp>atoi(PQ_rcv[pqts]))
        {
            timestamp++;
        }
        else
        {
            timestamp=atoi(PQ_rcv[pqts]);
            timestamp++;
        }
        
        for(i=0;i<7;i++)
            printf("Received message : %s\n",PQ_rcv[i]);
        
        if(PQ_rcv[pqmsgtype][2]=='q')
        {
            printf("CRR:ITS A REQ MSG!\n");
            
            while(pq_mutex==1);
            pq_mutex=1;
            printf("CRR-q:Mutex obtained\n");
            int i;int index;
            
             
            if(atoi(PQ_rcv[pqts])>timestamp)
            {
                if(clntSocket==sock_saver[0])
                    no_of_replies0++;
                else if(clntSocket==sock_saver[1])
                    no_of_replies1++;
                else if(clntSocket==sock_saver[2])
                    no_of_replies2++;
                else if(clntSocket==sock_saver[3])
                    no_of_replies3++;
            }
           
            for(i=pq_read_point;i<=pq_write_point;i++)
            {
                index=i;
              if(atoi(PQ_rcv[pqts])<atoi(PQ[i][pqts]))
              {
                  index=i;break;  
              }
            }
           printf("Insert point : %d",index);
            char temp[7][100];
            char temp_send[7][100];
            
            int j;
            
            for(j=0;j<7;j++)
            {
            strcpy(temp_send[j],PQ_rcv[j]);
            }
                
            for(i=index;i<=pq_write_point;i++)
            {
                
                    for(j=0;j<7;j++)
                    {
                        strcpy(temp[j],PQ[i][j]);
                    }
                    for(j=0;j<7;j++)
                    {
                        strcpy(PQ[i][j],PQ_rcv[j]);
                    }
                    for(j=0;j<7;j++)
                    {
                        strcpy(PQ_rcv[j],temp[j]);
                    }
            
            }
            
            for(j=0;j<7;j++)
            {
                printf("Inserted : %s\n",PQ[index][j]);   
            }
            
            
            pq_write_point++;
            
            strcpy(temp_send[pqmsgtype],"rep");
            int recvMsgSize=sizeof(temp_send);
            timestamp++;
            if (send(clntSocket, temp_send, recvMsgSize, 0) != recvMsgSize)
                           ErrorHandler("send()3 failed");
            
            pq_mutex=0;
            printf("CRR-q:Mutex released\n");
            
        }
        else if(PQ_rcv[pqmsgtype][2]=='p')
        {
            printf("CRR:ITS A REP MSG!\n");
            
              if(atoi(PQ_rcv[pqts])>timestamp)
              {
                  if(clntSocket==sock_saver[0])
                      no_of_replies0++;
                  else if(clntSocket==sock_saver[1])
                      no_of_replies1++;
                  else if(clntSocket==sock_saver[2])
                      no_of_replies2++;
                  else if(clntSocket==sock_saver[3])
                      no_of_replies3++;
                  
                  
              }
        }
        else if(PQ_rcv[pqmsgtype][2]=='l')
        {
            printf("CRR:ITS A REL MSG!\n");
            while(pq_mutex==1);
            pq_mutex=1;
            printf("CRR-l:Mutex obtained\n");
            int i;int index;
         
             
             if(atoi(PQ_rcv[pqts])>timestamp)
             {
                 if(clntSocket==sock_saver[0])
                     no_of_replies0++;
                 else if(clntSocket==sock_saver[1])
                     no_of_replies1++;
                 else if(clntSocket==sock_saver[2])
                     no_of_replies2++;
                 else if(clntSocket==sock_saver[3])
                     no_of_replies3++;
                 
                 
             }
          
            for(i=pq_read_point;i<pq_write_point;i++)
            {
                int len=strlen(PQ_rcv[pqip]);
                if(strncmp(PQ_rcv[pqip],PQ[i][pqip],len)==0)
                {
                    printf("CRR:same ip\n");
                   if(strncmp(PQ_rcv[pqport],PQ[i][pqport],4)==0)
                   {  
                       printf("CRR:same port : %s\n",PQ_rcv[pqport]);
                       int flen=strlen(PQ_rcv[pqfname]);
                        printf("File names::%s | %s\n",PQ_rcv[pqfname],PQ[i][pqfname]);
                       if(strncmp(PQ_rcv[pqfname],PQ[i][pqfname],8)==0)
                       {
                             printf("CRR:Got the file to be released\n");
                           index=i;break;  
                         
                       }
                   }}
            }
            
        
            for(i=index;i<=pq_write_point;i++)
            {   
                int j;
                for(j=0;j<7;j++)
                {
                strcpy(PQ[i][j],PQ[i+1][j]);
                }
            }
            pq_write_point--;
        }
            printf("CRR-l Write_point val : %d\n",pq_write_point);
            pq_mutex=0;
            printf("CRR-l:Mutex released\n");
         
        
        
            
        //chk if rep or req and add it to the qqueue
    }
    
    close(clntSocket);    /* Close client socket */
}
