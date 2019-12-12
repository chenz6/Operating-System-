/*********************************************************************
// --------- ----------- almost let me died  ----------- -------------
//  main.c
//  buildroom
//  chenz6@oregonstate.edu
//  Created by Zhuoling chen on 2018/2/08.
//  Copyright © 2018年 Zhuoling chen. All rights reserved.
 *********************************************************************/

#include<stdio.h>
#include<string.h>
#include<time.h>
#include <dirent.h>
#include <pwd.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>


/*********************************************************************
*Define
 *********************************************************************/
typedef int bool;
#define true 1
#define false 0

#define ROOMLIST 10
#define ROOMUSE  7
#define BUFFER_LEN 12

/*********************************************************************
 *Name: roomname
 *Function:Seting room name
 *Element: 10
 *********************************************************************/

char *roomname[ROOMLIST]={
    "Living",
    "Kitchen",
    "Study",
    "Gaming",
    "Bath",
    "Elevator",
    "Bed",
    "Hallway",
    "Stair",
    "Balcony"
};

/*********************************************************************
 *Name roomtype
 *Function Seting room type
 *Element 3
 *********************************************************************/

char *roomtype[3]={
    "start_room",
    "mid_room",
    "end_room"
};

/*********************************************************************
 *Function:message of whole room
 *********************************************************************/

struct room{
    char *roomname;
    char *roomtype;
    unsigned connum;
    unsigned a;// capacity conneting room
    struct room *link[ROOMUSE];
};

/*********************************************************************
 *Name:/
 *Function: announced all function
 *Element:/
 *********************************************************************/

struct room list[ROOMUSE];
struct room *build();
int create_rooms_dir(char *dir_name);
void connectRadom();
bool Connecroom(unsigned room1,unsigned room2);
bool alreadyconn(unsigned int room1,unsigned room2);
void sequence(struct room rooms[ROOMUSE]);
void printconnecon(struct room *r);
char *getfile();
struct room desequence(char *roomname);
char *rightname(char *in);
struct room *rightroom(char *in);
void destroy_rooms(struct room *rooms);

///*********************************************************************
// *link room strcut
// *********************************************************************/

struct linkroom{
    int roomord;
    struct room *coonect_room;
};

/*********************************************************************
 Name: building room
 Including : 4 functions
            -build
                -connectRadom
                    -connectroom
                        -alreadyconn
*********************************************************************
 
 
*********************************************************************
 *Name: build
 *Using for loop to build room
        after built choose one of room to give a name
                    if can not excute random, just give name by order
 *giving a room type[mid]
 *********************************************************************/

struct room *build(){
    int i;
    bool given[ROOMUSE];
    memset(&given, 0, ROOMUSE* sizeof(bool));
    
    for(i=0;i<7;i++){
        list[i].connum=0;
        unsigned a=rand()%(6-3);
        a+=3-2;
        list[i].a=a;

        while(1){
            unsigned roomord=rand()% ROOMUSE;
            if (!given[roomord]){
                given[roomord]=true;
                list[i].roomname=roomname[roomord];
                break;
            }

        }
        if( i == 0 )
            list[i].roomtype = "start_room";
        else if( i == 6)
            list[i].roomtype = "end_room";
        else
            list[i].roomtype = "mid_room";
    }
    connectRadom();
    
    return list;
}

/*********************************************************************
 *Name:ConnectRadom
 *Function:random pick room to connected
 *********************************************************************/

void connectRadom(){
    int i;
    int j;
    for (i = 0; i < 7; i++) {
        for (j = 0; j < list[i].a; j++) {
            unsigned int random_room = rand() % ROOMUSE;
            while (!Connecroom(i, random_room)) {
                random_room = rand() % ROOMUSE;
            }
        }
    }
}


/*********************************************************************
 *Name:Connecroom
 *Function: connected two rooms
 *********************************************************************/
bool Connecroom(unsigned room1,unsigned room2){
        struct room *r1= &list[room1];
        struct room *r2= &list[room2];
            if (r1->connum == 6) {
                return true;
            }
    
            if (alreadyconn(room1, room2)) {
                return false;
            }
    
            if (r1->connum >= 6|| r2->connum >= 6) {
                return false;
            }
    assert(r1 != NULL);
    assert(r2 != NULL);
    r1->link[r1->connum] = r2;
    r2->link[r2->connum] = r1;
    r1->connum++;
    r2->connum++;
    assert(r1->link[r1->connum-1] != NULL);
    assert(r2->link[r2->connum-1] != NULL);
    return true;
}

/*********************************************************************
 *Name:alreadyconn
 *Function: judgement already connected or not 
 *********************************************************************/
bool alreadyconn(unsigned room1,unsigned room2){
    int i;
    if (room1 == room2){
        return true;
    }
    
        for ( i= 0; i <list[room1].connum; i++) {
            if (list[room1].link[i] == &list[room2] &&
                list[room1].link[i] != NULL) {
            return true;
            }
        }
    return false;
}

/*********************************************************************
                        End building room -yep!
 *********************************************************************
 
  *********************************************************************
           start a newfunction~~~~
            Start to connect rooms
  *********************************************************************
 
 *********************************************************************
 Name:sequence
 Including 2 Function
            -sequence
                -getfile
                    -printconnecon
                        -desequence
                            -rightname
                                -rightroom
 *********************************************************************/


 /*********************************************************************
 *Name:sequence
 *********************************************************************/
void sequence(struct room rooms[ROOMUSE]){
    int i,j;
    char *dir_name = getfile();
    mkdir(dir_name, 0777);
    chdir(dir_name);

    for (i = 0; i < ROOMUSE; i++) {
        FILE *fp = fopen(list[i].roomname, "w");
        fprintf(fp, "room name : %s\n", list[i].roomname);
        
        for (j = 0; j < list[i].connum; j++) {
            fprintf(fp, "connected %d: %s\n", j + 1, list[i].link[j]->roomname);
        }
        
        fprintf(fp, "room type: %s\n", list[i].roomtype);
    }
    chdir("..");
}
 
 /*********************************************************************
 *Name:get file
 *Function:get file from strcut of room
 *********************************************************************/
char *getfile(){
        
        pid_t pid = getpid();
        
        uid_t uid = getuid();
        struct passwd *user = getpwuid(uid);
        
        unsigned long buffer = strlen(".rooms.") + strlen(user->pw_name) + 10;
        char *dir_name = malloc(buffer * sizeof(char));
        assert(dir_name != NULL);
        sprintf(dir_name, "%s.rooms.%d", user->pw_name, pid);
        return dir_name;
    }

/*********************************************************************
 *Name:printconnecon
 *Function:Print all connections for the user.
*********************************************************************/
void printconnecon(struct room *r){
    int i;
    printf("possible connection: ");
        for (i = 0; i < r->connum-1; i++) {
            printf("%s, ", r->link[i]->roomname);
        }
        if (r->connum > 0) {
            printf("%s.\n", r->link[r->connum-1]->roomname);
        }
    }
/*********************************************************************
 Name:desequence
 Function:Deserialize a single room.
*********************************************************************/
struct room desequence(char *roomname){
        struct room r;
        FILE *file = fopen(roomname, "r");
        char received_name[BUFFER_LEN];
        fscanf(file, "ROOM NAME  : %s\n", roomname);
        r.roomname = rightname(roomname);
        
        int read;
        int conn_number;
        while ((read =
                fscanf(file, "CONNECTION %d: %s\b", &conn_number, received_name)) != 0&& read != EOF) {
            r.link[conn_number-1] = rightroom(received_name);
        }
        r.connum =  conn_number - 1;
        fscanf(file, "ROOM TYPE: %s\n", received_name);
        if (strcmp(roomname, "start_room") == 0) {
            r.roomtype ="start_room";
        }
        else if (strcmp(roomname, "end_room") == 0) {
            r.roomtype = "end_room";
        }
        else
            r.roomtype = "mid_room";
        
        fclose(file);
        return r;
    }
/*********************************************************************
 *Name:rightname
 *Function:Check if the name is correct.
*********************************************************************/
char *rightname(char *in) {
        
        int i;
        
        for (i = 0; i < 6; i++) {
            if (strcmp(in, roomname[i]) == 0) {
                return roomname[i];
            }
        }
        return NULL;
    }
/*********************************************************************
*Name:rightroom
*Check if the room is correct.
*********************************************************************/
    struct room *rightroom(char *in) {
        
        int i;
        
        for (i = 0; i < ROOMUSE; i++) {
            if (strcmp(in, list[i].roomname) == 0) {
                return &list[i];
            }
        }
        return NULL;
    }
/*********************************************************************
                end connect part also including judgement
*********************************************************************
/*********************************************************************
main
*********************************************************************/

int main(){
    
    srand( (unsigned )time( NULL ) );

    build();
    
    sequence(list);

    return 0;
}


