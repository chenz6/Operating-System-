//
//  otp_enc_d.c
//  OTP
//
//  Created by Zhuoling chen on 2018/3/15.
//  Copyright © 2018年 Zhuoling Chen. All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

/*********************************************************************
 *Name:error
 *Function:return error
 *********************************************************************/
void error(const char *msg) {
    perror(msg);
    exit(1);
}

/*********************************************************************
                    list function
 *********************************************************************/

unsigned int getFiles(int socketFD, char **key, char **plainText);

pid_t encryptFork(int socketFD);

char* encryptChars(char *plainText, char *key, unsigned int size);

char modularAddition(char a, char b);

int writeToClient(int socketFD, char *cipherText, int size);

/*********************************************************************
                        main
 *********************************************************************/

int main(int argc, char *argv[])
{
   
    int listenSocketFD, establishedConnectionFD, forkSocketFD, forkPort, port, n;
    socklen_t sizeOfClientInfo;
    char buffer[256];
    char port_str[6];
    struct sockaddr_in serverAddress, forkServerAddress, clientAddress;
  
    srand(time(0));
    

    if (argc < 2) // shhould input number
    {
        fprintf(stderr, "Incorrect syntax");
        
        exit(0);
    }
    
  
    port = atoi(argv[1]);  //port number->integer
    
  
    if(port < 50000) //port should >50000
    {
        fprintf(stderr, "ERROR: must pick a port > 50000" );
        
        exit(1);
    }
    
    
    listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);//check socket for the listening port
    
    if (listenSocketFD < 0)
    {
        error("ERROR opening socket");
    }
    
 
    forkSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    
    if (forkSocketFD < 0)
    {
        error("ERROR opening socket");
    }
    
    
    memset((char *) &serverAddress, '\0', sizeof(serverAddress));//sever information
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    serverAddress.sin_port = htons(port);
    
    
    memset((char *) &forkServerAddress, '\0', sizeof(forkServerAddress));//forked connection
    forkServerAddress.sin_family = AF_INET;
    forkServerAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    

    if (bind(listenSocketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
    {
        error("ERROR on binding");
    }
    
  
    listen(listenSocketFD,5);
    
   
    sizeOfClientInfo = sizeof(clientAddress); //client adress length
    
  
    while(1)
    {
        
        establishedConnectionFD = accept(listenSocketFD,(struct sockaddr *) &clientAddress, &sizeOfClientInfo);
        

        if (establishedConnectionFD < 0)
        {
            error("ERROR on accept");  //accept data have error
        }
        
        
        do
        {
            forkPort = rand() % 65536; //try a random port until it can be used
            
            forkServerAddress.sin_port = htons(forkPort);
        
        }while(bind(forkSocketFD, (struct sockaddr *) &forkServerAddress, sizeof(forkServerAddress)) < 0);

       
        listen(forkSocketFD,1);
        
    
        sprintf(port_str, "%i", forkPort);
        
        n = write(establishedConnectionFD, port_str, strlen(port_str)); // Send the new port to the client to connect
        
      
        if (n < 0)
        {
            error("ERROR sending new port to client");
        }
        
        
        
        encryptFork(forkSocketFD);
    
        close(forkSocketFD);

        forkSocketFD = socket(AF_INET, SOCK_STREAM, 0);//new scok for new connection
        
    
        if (forkSocketFD < 0)
        {
            error("ERROR opening socket");
        }
    }
    
   
    close(listenSocketFD);
    
    return 0;
    
}

/*********************************************************************
*Name:getFiles
*Function:find txt and key file
*********************************************************************/

unsigned int getFiles(int socketFD, char **_key, char **_plainText)
{
    
    char buffer[100];// file sizes in buffer
    
  
    unsigned int keySize, plainTextSize, i = 0, n;
    
    
    n = recv(socketFD, buffer, 2, 0); // Receive the first two characters to determine if we need to encrypt or decrypt
    
    if(n < 0)
    {
        error("ERROR reading from socket");
    }
    
    
   
    if (buffer[0] == '1')
    {
        return -1; // If the first character is "1", client wants decryption.  return -1
    }
    
    
  
    do
    {
        n = recv(socketFD, buffer+i, 1, 0);
        if (n < 0)
        {
            error("ERROR reading from socket");
        }
        
        i++;
    }
    while (buffer[i-1] != '\n');
    

    buffer[i-1] = '\0'; //add \0 in the end
    
   
    keySize = atoi(buffer);
    char *key = malloc(keySize);
    

    *_key = key;//a point for address
    
 
    i = 0;//sent key file
    
    do
    {
        n = recv(socketFD, key + i, 1, 0);
        
        if (n < 0)
        {
            error("ERROR reading from socket");
        }
        
        i++;
    }
    while (key[i-1] != '\n');
    i = 0;
    
    buffer[0] = 0;
    
    do
    {
        n = recv(socketFD, buffer + i, 1, 0);
    
        if (n < 0)
        {
            error("ERROR reading from socket");
        }
        
        i++;
    }
    while (buffer[i-1] != '\n');
    
   
    buffer[i-1] = '\0';
    
    plainTextSize = atoi(buffer);
    
    char *plainText = malloc(keySize);
    
  
    *_plainText = plainText;  // Set the input argument to point to this address
    
    
    
    i = 0;
    
    do
    {
        n = recv(socketFD, plainText + i, 1, 0);
    
        if (n < 0)
        {
            error("ERROR reading from socket");
        }
        
        i++;
    }
    while (plainText[i-1] != '\n');
    
    return plainTextSize; // Return the size
}


/*********************************************************************
 *Name:encryptFork
 *Function:key file and txt file
 *********************************************************************/

pid_t encryptFork(int socketFD)
{
    int n;
    
   
    pid_t pid = fork();
    
    char buffer[2];

    if (pid > 0)
    {
        return pid;
    }
    
    else if (pid == 0)// Process is the child
    {
       
        char *key;
        char *plainText;
        char *encryptedText;
        unsigned int size;
        
        struct sockaddr_in clientAddress;
        socklen_t clientLength;
        clientLength = sizeof(clientAddress);
        
        socketFD = accept(socketFD, (struct sockaddr *) &clientAddress, &clientLength);

        
        size = getFiles(socketFD, &key, &plainText);
        
   
        if (size == -1)
        {
            
            sprintf(buffer,"0\n", 2);
            
            n = send(socketFD, buffer, 2, 0);
            
          
            if (n < 2)
            {
                error("ERROR writing to socket");
            }
            
            
            printf("ERROR: otp_dec cannot use otp_enc_d\n");
            
            
            close(socketFD);
            
            exit(0);
        }
        else
        {
            
            sprintf(buffer,"1\n", 2);
            
            n = send(socketFD, buffer, 2, 0);
            
            
            if (n < 2)
            {
                error("ERROR writing to socket");
            }
            
        }
      
        encryptedText = encryptChars(plainText, key, size);  // Encrypt the plainText using the key
       
        writeToClient(socketFD, encryptedText, size); // Send the encrypted text back to the client
        
        close(socketFD);
        
        exit(0);
    }
    

    else if (pid < 0)
    {
        fprintf(stderr,"Encryption fork failed\n");
    }
    
    return pid;
}


/*********************************************************************
 *Name: encryptChars
 *Function:The plaintext file is encrypted with the given key.
 *********************************************************************/
char* encryptChars(char *plainText, char *key, unsigned int size)
{
    
    int i, n;
    char *encryptedText;
    
    encryptedText = malloc(size);
    
    encryptedText[size-1] = '\n';
  
    for (i = 0; i < size-1; i++)
    {
        encryptedText[i] = modularAddition(plainText[i], key[i]);
    }
    

    return encryptedText;
}



/*********************************************************************
 *Name:modularAddition
 *Function:  the modular addition of two chars
 *********************************************************************/
char modularAddition(char a, char b)
{
    char c;
   
    
    if (a == 32)
    {
        a = 26;
    }
    else
    {
        a -= 65;
    }
    
    
    if (b == 32)
    {
        b = 26;
    }
    else
    {
        b -= 65;
    }
    
 
    c = a + b;   // Add the two chars
    
   
    if (c >= 27)
    {
        c -= 27;
    }
    
    if (c == 26)
    {
        return 32;
    }
    else
    {
        return c + 65;
    }
}



/*********************************************************************
 *Name:writeToClient
 *Function: give the encryted data back to the client
 *********************************************************************/
int writeToClient(int socketFD, char *encryptedText, int size)
{
    // Variable declarations
    int n, remaining, packetSize, totalLeft, outTotal = 0;
    
    // Continue sending packets of 1000 bytes until no more data
    while (outTotal < size)
    {
        totalLeft = 0;
        
        // If there is more than 1000 characters remaining to be sent, send a packet of 1000 bytes
        if (size - outTotal > 1000)
        {
            packetSize = 1000;
        }
        else
        {
            packetSize = size - outTotal;
        }
        
        remaining = packetSize;//reset
        
    
        while (totalLeft < packetSize)
        {

            n = send(socketFD, encryptedText + outTotal, remaining, 0);

            if (n == -1)
            {
                return outTotal;
            }
            
            // Increase totalLeft and outTotals
            totalLeft += n;
            outTotal += n;
            
            // Decrement the remaining
            remaining -= n;
        }
    }
    
    return outTotal;
}


