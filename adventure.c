
//  main.c
//  adventure
//
//  Created by Zhuoling chen on 2018/2/14.
//  Copyright © 2018年 Zhuoling Chen. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

/*********************************************************************
 *Define
 *********************************************************************/
#define SIZE  256
#define ROOMUSE 7
#define MAXCON 6

pthread_mutex_t timeobject;

enum Roomtype{
    start_room,
    mid_room,
    end_room
};
/*********************************************************************
 *Name: room
 *Function:message of room
 *Element:
 *********************************************************************/
struct room{
    char *roomname;
    enum Roomtype roomtype;
    unsigned connum;
    struct room *joint[MAXCON];
};

/*********************************************************************
 *Name:/
 *Function: announced all function
 *Element:/
 *********************************************************************/

struct room *roomar;

void look(char *find, char *first, char *i);
void getrooms(char *dir,struct room *roomarr);
void initlist(struct room *list[]);
struct room *initempty();
void  getall(char* dir,struct room *roomarr);
void sprestr(char *i, char *j, char *k);
enum Roomtype getroomtypenum(char *roomtype);
int getroomname(char *roomname,struct room *roomarr);
int connectalea(struct room *room1, struct room *room2);
void connectrom(struct room *room1, struct room *room2);
int getroombtype(enum Roomtype roomtype, struct room *roomarr);
void printroom(struct room *room);
void inputahand(char *input);
void printtime();
int timethread();
void *createTime();
int canmove(char *roomName, struct room *room);
int getroombname(char *roomName, struct room *roomarr);
/*********************************************************************
*Name: look
Including 8 function
 -look
 -getrooms
 -initlist
 -initempty
 -getall
 -connectalea
 -connectrom
 -getroomtypenum
*********************************************************************/
/*********************************************************************/
void look(char *find, char *first, char *i){
    int dirtime = -1;
    memset (i,'\0',sizeof(* i));
    
    DIR * dirfirst;
    struct dirent *file;
    struct stat dir_chara;
    dirfirst = opendir (find);
    if (dirfirst > 0 ) {
        while((file = readdir(dirfirst)) != NULL){
            if (strstr(file->d_name, first) !=NULL) {
                    stat(file->d_name, &dir_chara);
                if ((long int)dir_chara.st_mtime > dirtime){
                    dirtime =(long int)dir_chara.st_mtime;
                    memset(i, '\0',sizeof(*i));
                    strcpy(i, "./");
                    strcat(i, file->d_name);
                }
            }
        }
        closedir(dirfirst);
    }
    
}
/*********************************************************************
 *Name:initialRoomArr
 *Fuction:give space to every room
/*********************************************************************/
struct room* initialRoomArr(){
    
    struct room* rm = malloc(sizeof(struct room)*7);
    int i;
    for( i = 0; i< 7;i++)
    {
        rm[i].roomname = malloc(sizeof(char) * 20);
        rm[i].connum = 0;
    }
    
    return rm;
}

/*********************************************************************
 *Name:getrooms
 *Function: get document of room
/*********************************************************************/
void getrooms(char *dir,struct room *roomarr){
    int i,j;
    DIR *dirfirst;
    size_t len;
    ssize_t read;
    FILE *roomfile;
    char buff[SIZE];
    char value[SIZE], *line = NULL, *val;
    getall(dir,roomarr);
   
    // Open up the directory this program was run in and move to it
    for(i = 0; i < 7; i++)
    {
        char *dirr = malloc(256 * sizeof(char));
        sprintf(dirr, "%s/%s", dir, roomarr[i].roomname);

        
        roomfile = fopen( dirr, "r");
        
        while ((read = getline(&line, &len, roomfile)) != -1) {
            if(strstr(line,"connect") != NULL)
            {
                val = strtok(line," :");
                val = strtok(NULL," :\n");
                val = strtok(NULL," \n");
                j = getroomname(val, roomarr);
                if(connectalea(&roomarr[i], &roomarr[j]) != 1)
                    connectrom(&roomarr[i], &roomarr[j]);
            
            }
            else
            {
                val = strtok(line,":");
                val = strtok(NULL," \n");
                roomarr[i].roomtype =getroomtypenum(val);
            }
        }

    }
    
}

/*********************************************************************
 *Name:getall
 *Function: get all rooms name
            find all file and set in array
/*********************************************************************/
void  getall(char *dir,struct room *roomarr){
    DIR* dirfirst;
    struct dirent *dirins;
    int numRoom = 0;
    
    dirfirst = opendir(dir);
    if ( dirfirst >0) {
        while ((dirins = readdir(dirfirst)) != NULL) {
            if(strlen(dirins->d_name) > 2){
                strcpy(roomarr[numRoom].roomname, dirins->d_name);
                numRoom++;
            }
        }
        closedir(dirfirst);
    }
    
    free(dirins);
}
/*********************************************************************
 *Name: sprestr
 *Function Separte the strings
/*********************************************************************/
void sprestr(char *i, char *j, char *k){
    strtok(j,i);
    strcpy(k,strtok(NULL,""));
    k[strlen(k)] = '\0';
    j[strlen(j)] = '\0';
    int n;
    for(n = 0;n < strlen(k);n++){
        k[n] = k[n+1];
    }
    k[strcspn(k, "\n")] = '\0';
}


/*********************************************************************
 *Name:getroomtypenum
 *Functiom using enum get room name
/*********************************************************************/
enum Roomtype getroomtypenum(char *roomtype){
    if(strstr(roomtype, "start_room") != NULL){
        return start_room;
    }
    if(strstr(roomtype,"mid_room") != NULL){
        return mid_room;
    }
    if(strstr(roomtype,"end_room") != NULL){
        return end_room;
    }
    return 0;

}

/*********************************************************************
 *Name:getroomname
 *Function: return the whole room and give name
 /*********************************************************************/

int getroomname(char *roomname,struct room *roomarr){
    int i;
    for (i = 0; i < ROOMUSE; i++) {
        if(strcmp(roomarr[i].roomname, roomname) == 0){
            return i;
        }
    }
    return -1;
}

/*********************************************************************
 *Name:getroomtypenum
 *Function using enum get room name
/*********************************************************************/
int connectalea(struct room *room1, struct room *room2){
    int i;
    for(i = 0; i < room1->connum; i++){
        if(room1->joint[i] == room2){
            return 1;
        }
    }
    return -1;
}
/*********************************************************************
 *Name:connectrom
 *Function connect 2 rooms
/*********************************************************************/
void connectrom(struct room *room1, struct room *room2){
    room1->joint[room1->connum] = room2;
    room2->joint[room2->connum] = room1;
    room1->connum++;
    room2->connum++;
}
/*********************************************************************
 *Name:star
 *Function start game
 including
 -getroombtype
 -printroom
 -inputahand
 -timethread
 - createTime(
 -printtime
 -canmove
 -getroombname
/*********************************************************************/
void start(struct room *roomarr){
    char *input=malloc(sizeof(char)*160);
    int pathTaken[1024];
    int stepCount = 0;
    int curPos = getroombtype(start_room, roomarr);
    struct room *curRoom = &roomarr[curPos];
    
    while(curRoom->roomtype != end_room){
        printroom(curRoom);

        inputahand(input);
        if(canmove(input,curRoom) == 1){
            pathTaken[stepCount] = curPos;
            curPos = getroombname(input, roomarr);
            curRoom = &roomarr[curPos]; // Set current position to the new room
            stepCount++; // Increase Step Count
        } else {
            printf("HUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
        }
    }
    // Add Last Room
    pathTaken[stepCount] = curPos;
    
    // Print Ending Message!
    printf("\n*****\n");
    printf("* Congratulations! You made it to %s in %d step(s)! The path you took was:\n* ", roomarr[curPos].roomname, stepCount);
    // Print all rooms
    int i;
    for (i = 0; i <= stepCount; i++) {
        printf("%s", roomarr[ pathTaken[i] ].roomname);
        if(i != stepCount){
            printf("-->");
        }
    }
    printf("***********************************************************************");
}
/*********************************************************************
 get roomtype
/*********************************************************************/
int getroombtype(enum Roomtype roomtype, struct room *roomarr){
    int i;
    for (i = 0; i < ROOMUSE; i++) {
        if(roomarr[i].roomtype == roomtype){
            return i;
        }
    }
    return -1;
}
/*********************************************************************
 printroom
/*********************************************************************/
void printroom(struct room *room){
    
    printf("CURRENT LOCATION: %s\n", room->roomname);
    printf("POSSIBLE CONNECTIONS: ");
    int x;
    for (x = 0; x < room->connum; x++) {
        printf("%s", room->joint[x]->roomname);
        if((x+1) == room->connum){
            printf(".\n");
        } else {
            printf(", ");
        }
    }
}
/*********************************************************************
 inputahand
 Use loop because of format.
 If a user types "time", mentioned that the program should return
 the time and then re-prompt with "WHERE TO? >"
/*********************************************************************/
void inputahand(char *input){
    while(1){
        memset(input,'\0',sizeof(&input));
        printf("WHERE TO? >");
        scanf("%255s", input);
        printf("\n");
        if(strcmp(input, "time") == 0){
            if( timethread() == 1){
                printtime();
            }
        } else {
            break;
        }
    }
}
/*********************************************************************
 timethread
/*********************************************************************/
int timethread(){
    pthread_t createTime_THREAD;
    pthread_mutex_lock(&timeobject);
    
    if(pthread_create(&createTime_THREAD,NULL,createTime,NULL) != 0){
        printf("ERROR DURING CREATE TIME FILE THREAD!");
        exit(1);
    }
    
    pthread_mutex_unlock(&timeobject);
    pthread_join(createTime_THREAD,NULL);
    
    return 1;
}
/*********************************************************************
  createTime
 from current time created time cacluated
/*********************************************************************/
void * createTime(){
    char timeHolder[SIZE];
    time_t currTime;
    struct tm * timeInfo;
    FILE *timeFile;
    memset(timeHolder,'\0',sizeof(timeHolder));
 
    time(&currTime);
    timeInfo = localtime(&currTime);
  
    strftime(timeHolder,SIZE, "%I:%M%P %A, %B %d, %Y", timeInfo);

    timeFile = fopen("currentTime.txt","w");
    fprintf(timeFile,"%s\n",timeHolder);
    fclose(timeFile);
    return NULL;
}
/*********************************************************************
 print time
/*********************************************************************/
void printtime(){
    FILE* timeFile;
    char *fileBuff;
    fileBuff = malloc(sizeof(char)*SIZE);
    memset(fileBuff, '\0', SIZE);
    timeFile = fopen("currentTime.txt", "r");
    if(timeFile != NULL){
        fgets(fileBuff, SIZE, timeFile);
        printf("%s\n", fileBuff);
        fclose(timeFile);
    }
    else {
        printf("ERROR ACCESSING: currentTime.txt");
        exit(1);
    }
    // clean up
    free(fileBuff);
}
/*********************************************************************
 Name:canmove
 Function: verifies name entered matches, one of the names of the connections.
/*********************************************************************/
int canmove(char *roomName, struct room *room){
    int i;
    for (i = 0; i < room->connum; i++) {
        if(strcmp(room->joint[i]->roomname, roomName) == 0){
            return 1;
        }
    }
    return 0;
}
/*********************************************************************
 get room by name
/*********************************************************************/
int getroombname(char *roomName, struct room *roomarr){
    int i;
    for (i = 0; i < ROOMUSE; i++) {
       
        if(strcmp(roomarr[i].roomname, roomName) == 0){
            return i;
        }
    }
    return -1;
}
/*********************************************************************
 main
/*********************************************************************/

int main() {
    char * newdir=malloc(sizeof(char)*256);
    look(".", "chenz6.rooms", newdir);
    struct room *rooms = initialRoomArr();
    getrooms(newdir,rooms);
    
    
    
    start(rooms);

    
    
}
