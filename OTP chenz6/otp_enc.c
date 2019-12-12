//
//  otp_enc.c
//  OTP
//
//  Created by Zhuoling chen on 2018/3/15.
//  Copyright © 2018年 Zhuoling Chen. All rights reserved.
//

/*
 Description: This is the encryption program that sends the plantext and key to the encryption server 
 to cipher and later sends it back
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


const int MAXCHARS = 1024;

/*********************************************************************
 *Name: error
 *Function: return file size
 *elements: size
 *cite from http://www.cplusplus.com/reference/cstdio/perror/
 *********************************************************************/

void error(const char *msg){
    perror(msg);
    
    exit(0);
    
}


/*********************************************************************
                    * list functions
 *********************************************************************/


int findFileSize(char* file);

int checkFile(char *file);

int sendFile(int socketFD, char *file);


/*********************************************************************
                            *Name: main
 *********************************************************************/

int main(int argc, char *argv[])
{
    
    int socketFD, port, n;
    struct sockaddr_in serverAddress;
    struct hostent *serverHostInfo;
    char buffer[100];
    char portno[6];
    char *keyFile;
    char *plainTextFile;
    
    
    if (argc < 4)
    {
        fprintf(stderr,"ERROR: Incorrect \n");// at least four commands entered
        exit(1);
    }
    
  
    keyFile = argv[2];  // have name of the key file


    plainTextFile = argv[1]; // plaintext file
    
   
    port = atoi(argv[3]);// port number
    

    if (port < 50000)    // port number should be 50000
    {
        fprintf(stderr, "Port number be > 50000\n");
        exit(2);
    }
    
   
   
    int plainTextFileSize = findFileSize(plainTextFile);// size( key file )= size( plaintxt )
    
    int keyFileSize = findFileSize(keyFile);
    
    if (plainTextFileSize > keyFileSize)
    {
        printf("ERROR: Key file is too-short \n");
        
        exit(1);
    }
    
   
    if (checkFile(keyFile) || checkFile(plainTextFile)) //  have invalid characters or not?
    {
       // printf( "ERROR:have documents contained an invalid character\n");
         fprintf(stderr, "ERROR: have documents contained an invalid character\n");
        exit(1);
    }
    
   
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    
 
    if (socketFD < 0)
    {
        error("cannot opening socket");// if socket was not opened correctly
    }

    
    serverHostInfo = gethostbyname("localhost");//put server information into localhost
    
   
    if (serverHostInfo == NULL)
    {
        fprintf(stderr,"ERROR, no such host\n");
        
        exit(0);
    }
    
    
    memset((char*)&serverAddress, '\0', sizeof(serverAddress));// set the struct of server address
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);
    
    

    if (connect(socketFD,(struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)    // start to connect to the server
    {
        error("ERROR connecting");
    }
    
    
    

    memset(portno, '\0', 6);
    
    n = read(socketFD, portno, 5);
    
    serverAddress.sin_port = htons(atoi(portno));
    
 
    close(socketFD);
    
    
    socketFD = socket(AF_INET, SOCK_STREAM, 0);// open a new socket using the retrieved port
    
    
    if (connect(socketFD,(struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
    {
        error("ERROR connecting");
    }
    
    
    
    sprintf(buffer,"0\n", 2);
    
    n = send(socketFD, buffer, 2, 0);
    
  
    if (n < 2)
    {
        error("ERROR writing to socket");
    }
    
    
    sprintf(buffer,"%i\n", findFileSize(keyFile));//  server will receive file length and key
    
    n = send(socketFD, buffer, strlen(buffer), 0);
    
  
    if (n < 0)
    {
        error("ERROR writing to socket");
    }
    
    

    n = sendFile(socketFD, keyFile);//set keyfile
    
    if (n < 0)
    {
        error("ERROR writing to socket");
    }
    
    

    sprintf(buffer,"%i\n", findFileSize(plainTextFile));//server will receive plaintext length
    
    n = send(socketFD, buffer, strlen(buffer), 0);
    
    if (n < 0)
    {
        error("ERROR writing to socket");
    }
    
    n = sendFile(socketFD, plainTextFile);
    
    if (n < 0)
    {
        error("ERROR writing to socket");
    }
    
 
   
    
    n = recv(socketFD, buffer, 2, 0);//check everything was successful
    
    if (n < 2)
    {
        error("ERROR reading from socket");
    }

    
    if (buffer[0] == '0')//not allow it coonnet to decypher
    {
        close(socketFD);
        
        fprintf(stderr,"ERROR: cannot connect to the decypher server\n");
        
        exit(1);
    }
    
   
    while (n = recv(socketFD, buffer, 1, 0))
    {
        if (n < 0)
        {
            error("ERROR reading from socket");
        }
        
        printf("%c",buffer[0]);// right -> print
    }
    
    close(socketFD);
    
    return 0;
    
}


/*********************************************************************
 *Name: findFileSize
 *Function: return file size
 *elements: size
 *cite from: http://www.cplusplus.com/reference/cstdio/ftell/
 *********************************************************************/

int findFileSize(char *file)
{

    FILE *newfile = fopen(file, "r");
    
    
    if (newfile == 0)
    {
        fprintf(stderr, "ERROR: Cannot open file %s\n", file);// if can not open, error
        
        exit(1);
    }
    
    //return size of file
    fseek(newfile, 0L, SEEK_END);
    
    int size = ftell(newfile);

    fseek(newfile, 0L, SEEK_SET);
    
    fclose(newfile);
    
    return size;
}

/*********************************************************************
 *Name: checkFile
 *Function:only have space and upper characters in the file content
 if found not space and upper characters return true
 *elements: c
 *********************************************************************/
int checkFile(char *file)
{
   
    char c;
    FILE *newfile = fopen(file, "r");
    
    
    if (newfile == 0)
    {
        fprintf(stderr, "ERROR: Cannot open file %s\n", file);
        
        exit(1);
    }
    
 
    while((c = fgetc(newfile)) != '\n')   // read the first line until have the newline
    {
        if ((c < 65 || c > 90) && c != 32)
        {
            fclose(newfile);
            
            return 1;
        }
    }
    
    fclose(newfile);
    
    return 0;
}

/*

/*********************************************************************
 *Name: sendFile
 *Function:send the file contents to socket established
 *cite from:https://www.tutorialspoint.com/c_standard_library/c_function_fgets.htm
 *********************************************************************/

int sendFile (int socketFD, char *file)
{
    
    char buffer[1001];
    FILE *newfile;
    
    int n, charsRemaining, totalLeft, outTotal = 0;
    
    newfile = fopen(file, "r");
    
    
    while (fgets(buffer, 1001, newfile))// Send 1000 chars in one time
    {
       
        totalLeft = 0;// success transform number
        
       
        charsRemaining = strlen(buffer);// still need to be send number
        
        while (totalLeft < strlen(buffer))//keep sending the data until finished
        {
            // send data
            n = send(socketFD, buffer + totalLeft, charsRemaining, 0);
            
            
            if (n == -1)  // if here have error, just sent the number currently
            {
                fclose(newfile);
                
                return outTotal;
            }
            
            totalLeft += n;
        
            charsRemaining -= n;
        }
        
       
        outTotal += totalLeft; // change the bytes in the file at time
    }
    

    fclose(newfile);
    
    return outTotal;
}













