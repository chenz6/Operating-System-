//
//  keygen.c
//  OTP
//
//  Created by Zhuoling chen on 2018/3/15.
//  Copyright © 2018年 Zhuoling Chen. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*********************************************************************
 *Name: main
 *Function: create key with a space and need to write in a file
 *elements:length keyChars key i
 *********************************************************************/

int main(int argc, char * argv[])
{
    
    if(argc < 2)//if arguemets is less than 2 -> wrong
    {
        fprintf(stderr, "please intput keygen 20 \n");
        exit(1);
    }

    int length = atoi(argv[1]);//transfer to int type
    
    char keyChars[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    
    char key[length+1];
    
    srand(time(0));// Initialize random number generator.
    
    int i;
    
    for(i = 0; i < length; i++)
    {
        key[i] = keyChars[rand() % 27];//produce < 27 random number
    }
    
    key[length] = '\0';
    
    printf("%s\n", key);
    
    return 0;
}
