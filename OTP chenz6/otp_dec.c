//
//  otp_dec.c
//  OTP
//
//  Created by Zhuoling chen on 2018/3/15.
//  Copyright © 2018年 Zhuoling Chen. All rights reserved.
//


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>

/*********************************************************************
 *Name:error
 *********************************************************************/

void error(const char *msg){
    perror(msg);
    exit(0);
    
}

/*********************************************************************
                    list functions
 *********************************************************************/

int findFileSize(char* file);
int checkFile(char *file);
int sendFile(int sockfd, char *file);

/*********************************************************************
                        *Name:main
 *********************************************************************/


int main(int argc, char *argv[])
{
  
    int socketFD, port, n;
    struct sockaddr_in serverAddress;
    struct hostent *server;
    char buffer[100];
    char portno[6];
    char *keyFile;
    char *encryptedTextFile;
    
 
    if (argc < 4)
    {
        fprintf(stderr,"ERROR: Incorrect syntax \n");// at least four commands entered
        exit(0);
    }
    
    
    keyFile = argv[2]; // have name of the key file
    
   
    encryptedTextFile = argv[1];// plaintext file
    

    port = atoi(argv[3]);
   
    if (port < 50000)
    {
        fprintf(stderr, "Port number be > 50000\n");
        exit(2);
    }
    
 

    int plainTextSize = findFileSize(encryptedTextFile);// size( key file )= size( plaintxt )
    
    int keySize = findFileSize(keyFile);
    
    if (plainTextSize > keySize)
    {
        fprintf(stderr,"ERROR: Key and plaintext file size are not the same\n");
        exit(1);
    }
    

    if (checkFile(keyFile) || checkFile(encryptedTextFile))
    {
       printf( "ERROR: documents contained an invalid character\n");
        //fprintf(stderr, "ERROR: documents contained an invalid character\n");
        exit(1);
    }
    
 
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    
    
    if (socketFD < 0)
    {
        error("ERROR opening socket");
    }
    
    
    server = gethostbyname("localhost");
    

    if (server == NULL)
    {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
   
    memset((char *) &serverAddress, '\0' ,sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    memcpy((char *)server->h_addr, (char *)&serverAddress.sin_addr.s_addr, server->h_length);

    
    if (connect(socketFD,(struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
    {
        error("ERROR connecting");
    }
    
   
    memset(portno,'\0' , 6);
    
    n = read(socketFD, portno, 5);
    
    serverAddress.sin_port = htons(atoi(portno));
    
   
    close(socketFD);
   
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    
    
    if (connect(socketFD,(struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
    {
        error("ERROR connecting");
    }
    


    
  
    sprintf(buffer,"1\n");
    
    n = send(socketFD, buffer, 2, 0);
    

    if (n < 2)
    {
        error("ERROR writing to socket");
    }
    
   
    sprintf(buffer,"%i\n", findFileSize(keyFile));
    
    n = send(socketFD, buffer, strlen(buffer), 0);

   
    if (n < 0)
    {
        error("ERROR writing to socket");
    }
    
    
    // send the keyfile contents including the null terminator
    n = sendFile(socketFD, keyFile);
    
    if (n < 0)
    {
        error("ERROR writing to socket");
    }
    
    

    sprintf(buffer,"%i\n", findFileSize(encryptedTextFile));
    
    n = send(socketFD, buffer, strlen(buffer), 0);
    
    if (n < 0)
    {
        error("ERROR writing to socket");
    }
    
    n = sendFile(socketFD, encryptedTextFile);
    if (n < 0)
    {
        error("ERROR writing to socket");
    }
    
    
    // Retrieve the message returned by the server to indicate if everything was successful of there was an error
    n = recv(socketFD, buffer, 2, 0);
    
    if (n < 2)
    {
        error("ERROR reading from socket");
    }
    
    
    // Throw error is attempting to connect to the decypher server
    // rather than the connect server
    if (buffer[0] == '0')
    {
        close(socketFD);
        
     //   fprintf(stderr,"ERROR: cannot use otp_enc_d\n");
          printf("ERROR: cannot use otp_enc_d\n");
        
        exit(1);
    }
    
    // If no errors, then print out to stdout
    while (n = recv(socketFD, buffer, 1, 0))
    {
        if (n < 0)
            error("ERROR reading from socket");
        
        printf("%c",buffer[0]);
    }
    
    close(socketFD);
    
    return 0;
    
}




\
/*********************************************************************
 *Name: findFileSize
 *Function: return file size
 *elements: size
 *cite from: http://www.cplusplus.com/reference/cstdio/ftell/
 *********************************************************************/

int findFileSize(char *file)
{
    
    FILE *newfile = fopen(file, "r");
    
    // Throw error if problem opening file
    if (newfile == 0)
    {
        fprintf(stderr, "ERROR: Cannot open file %s\n", file);
        
        exit(1);
    }
    
    // Set file position to the end of the file
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
    
    // Throw error if there is a problem with the contents of the file
    if (newfile == 0)
    {
        fprintf(stderr, "ERROR: Cannot open file %s\n", file);
        
        exit(1);
    }
    
    // Read the first line of the file up to the newline
    while((c = fgetc(newfile)) != '\n')
    {
        if ((c < 65 || c > 90) && c != 32)
        {
            fclose(newfile);
            
            return 1;
        }
    }
    
    // Close the file
    fclose(newfile);
    
    return 0;
}



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
    
    // Sends data 1000 chars at a time
    while (fgets(buffer, 1001, newfile))
    {
   
        totalLeft = 0;
        
        charsRemaining = strlen(buffer);
        
        while (totalLeft < strlen(buffer))
        {
            
            n = send(socketFD, buffer + totalLeft, charsRemaining, 0);
            
           
            if (n == -1)
            {
                fclose(newfile);
                
                return outTotal;
            }
            
            totalLeft += n;
            
            charsRemaining -= n;
        }
        
         // Update number of bytes sent from the file
        outTotal += totalLeft;
    }
    
   
    fclose(newfile);
    
    return outTotal;
}











