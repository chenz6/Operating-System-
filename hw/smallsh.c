//
//  smallsh.c
//  smallsh
//
//  Created by Zhuoling chen on 2018/3/4.
//  Copyright © 2018年 Zhuoling Chen. All rights reserved.
//
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define STACKMAX 512
/*********************************************************************
*struct  InputObj
 *********************************************************************/
struct InputObj

{
    bool Background;
    char InputFile[128];
    char OutputFile[128];
    char Command[1024];
    int NumArgs;
    char *Arguments[512];
};



/*********************************************************************
 *Name:BackStackModi
 *Function:modified stack in backstage
 *********************************************************************/
struct BackStackModi
{
    int NumBackP;
    pid_t BackgroundP[STACKMAX];
};

/*********************************************************************
 *Some global variable
 *********************************************************************/
struct BackStackModi BackStack;
int GroundDataLast;
bool OnlyFore = false;



/*********************************************************************
 *Name: InitailPid
 *Function: Value of stack is -1
 *Element: i,BackStack.NumBackP, BackStack.BackgroundP
 *********************************************************************/
void InitailPid()
{
    int i;
    BackStack.NumBackP = -1;
    
    for(i = 0; i < 512; i++){
        BackStack.BackgroundP[i] = -1;
    }
}


/*********************************************************************
 *Name: AddPid
 *Function: Add pid into stack
 *Element:BackStack.NumBackP, BackStack.BackgroundP,processId
 *********************************************************************/
void AddPid(pid_t processId)
{
    BackStack.BackgroundP[++(BackStack.NumBackP)] = processId;
}



/*********************************************************************
 *Name: RemoveEarly
 *Function: If early background process end deleteprocessId
 *Element:i,a,BackStack.NumBackP, BackStack.BackgroundP,processId
 *********************************************************************/
void RemoveEarly(pid_t processId)
{
    int i;
    int a; //processId position
    for(i = 0; i< BackStack.NumBackP + 1;i++){
        if(BackStack.BackgroundP[i] == processId){
            a = i;
            break;
        }
    }
    
    for(i = a; i < BackStack.NumBackP + 1;i++){
        BackStack.BackgroundP[i] = BackStack.BackgroundP[i+1];
    }
    
    BackStack.NumBackP--;
    
}

/*********************************************************************
 *Name: TopPid
 *Function: display top of stack
 *********************************************************************/
pid_t TopPid()
{
    return BackStack.BackgroundP[BackStack.NumBackP];
}

/*********************************************************************
 *Name: DirectChageInHom
 *Function: user using the guide directory
 *********************************************************************/
int DirectChageInHom (char* InputBuffer)
{
    char* HomeDirectoryPath = getenv("HOME"); //gets home path.
    char NewPath[1028];
    
    InputBuffer[strlen(InputBuffer) -1] = '\0';
    
    if(strcmp(InputBuffer,"cd") == 0){
        if(chdir(HomeDirectoryPath) != 0){ // cannot find directory
            printf("Directory:%s not found.\n",HomeDirectoryPath);
            return 1;
        }
        return 0;
    }
    
    memset(NewPath,'\0',sizeof(NewPath));
    
    strtok(InputBuffer," "); // removing unnessary spacing
    strcpy(InputBuffer,strtok(NULL,""));
    
  
    if(InputBuffer[0] == '/'){
        sprintf(NewPath,"%s%s",HomeDirectoryPath,InputBuffer); // goto a specifed directory from home directory
    
    }
    else if(strcmp(InputBuffer,"..") == 0){ // go back a folder
        strcpy(NewPath,InputBuffer);
    
    }
    else if(strcmp(InputBuffer,"~") == 0){ // go back a folder
        strcpy(NewPath,HomeDirectoryPath);

    }
    else if(InputBuffer[0] == '.' && InputBuffer[1] == '/'){ // current directory
        sprintf(NewPath,"%s",InputBuffer);
      
    }
    else{
        sprintf(NewPath,"%s",InputBuffer); // goto directory from home

    }
    if(chdir(NewPath) != 0){ // cannot find directory
        printf("Directory:%s not found.\n",NewPath);
        return 1;
    }
    return 0;
}


/*********************************************************************
 *Name: LeadCharJud
 *Function: check :
                    &
                    <
                    >
                    protect additional arguements
 *********************************************************************/
bool LeadCharJud(char *str)
{
    bool IsSpecial = false;
    
    if(str == NULL){
        return true;
    }
    
    if(str[0] == '&'){ // check for bg.
        IsSpecial = true;
    }
    else if(str[0] == '<'){ // check for input char
        IsSpecial = true;
    }
    else if(str[0] == '>'){ // check for output char
        IsSpecial = true;
    }
    else if(str[0] == '#'){ // check for command char
        IsSpecial = true;
    }
    
    return IsSpecial;
}


/*********************************************************************
 *Name: ParseInput
 *Function: include all arguement
 *Element:
 *********************************************************************/
void ParseInput(char* InputBuffer,struct InputObj* Obj)
{
    char Buffer[1028];
    char *InputFileName;
    char *OutputFileName;
    char *Temp;
 
    
    Obj->NumArgs = 0;
    InputBuffer[strlen(InputBuffer) -1] = '\0'; // removed \n
    
    if(InputBuffer[strlen(InputBuffer) -1] == '&'){
        Obj->Background = true;
        InputBuffer[strlen(InputBuffer) -1] = '\0'; // remove char.
     
    }
    else{
        Obj->Background = false;

    }
    
    //command
    memset(Buffer,'\0',sizeof(Buffer)); // clear Buffer
    strcpy(Buffer,InputBuffer); //copy buffer
    strtok(Buffer," "); // grab only command part of input;
    strcpy(Obj->Command,Buffer); // take command place in new obj.
    
    
    //InputFile Name
    memset(Buffer,'\0',sizeof(Buffer));
    strcpy(Buffer,InputBuffer);
    InputFileName = strstr(Buffer,"<"); // grab everything after <
    if(InputFileName != NULL){
        memmove(InputFileName, InputFileName+2, strlen(InputFileName)); //place everything but "< " in string
        strtok(InputFileName," "); // cut off excess
        InputFileName[strlen(InputFileName)] = '\0'; // add end char
        strcpy(Obj->InputFile,InputFileName);
    }
    
    //OutputFile Name;
    memset(Buffer,'\0',sizeof(Buffer));
    strcpy(Buffer,InputBuffer);
    OutputFileName = strstr(Buffer,">"); // grab everything after >
    if(OutputFileName != NULL){
        memmove(OutputFileName, OutputFileName+2, strlen(OutputFileName));//place everything but "> " in string
        strtok(OutputFileName," "); // cut off excess
        OutputFileName[strlen(OutputFileName)] = '\0'; // add end char
        strcpy(Obj->OutputFile,OutputFileName);
        //printf("Outputfile is:%s\n",Obj->OutputFile);
    }
    
    //arguments
    memset(Buffer,'\0',sizeof(Buffer));
    strcpy(Buffer,InputBuffer);
    strtok(Buffer," "); // everything before first space
    
    Temp = strtok(NULL,""); // grab everyhting after first space.
    
    //printf("Temp Line:%s\n",Temp);
    if(LeadCharJud(Temp) == false){ // check if there are any args
        strcpy(Buffer,Temp);
        strtok(Buffer,"<>#"); // grab everything before args.
        
        strtok(Buffer," "); // cut space
        Obj->Arguments[0] = Buffer; // first arg.
        Obj->NumArgs = 1;
        Temp = strtok(NULL," "); // next arg
        while(Temp != NULL){
            
            Obj->Arguments[Obj->NumArgs] = Temp;// input all args into list.
            Obj->NumArgs++;
            Temp = strtok(NULL," ");
        }
        Obj->Arguments[Obj->NumArgs] = strtok(NULL, ""); // grab last arg.
        
    }
    

}



/*********************************************************************
 *Name:InitArg
 *Function: creates list about argument
 *********************************************************************/
void InitArgList(struct InputObj* Obj,char** Args)
{
    int i;
    Args[0] = Obj->Command; // first arg is command itself.
    for(i = 0;i < Obj->NumArgs ;i++){
        if(getenv(Obj->Arguments[i]) != NULL){
            Args[i+1] = getenv(Obj->Arguments[i]); // add all args.
        }
        else if(strcmp(Obj->Arguments[i],"$$")== 0){
            pid_t p = getpid();
            sprintf(Args[i+1], "%d", p);
          
//             Args[i] = replaceWord(Obj->Arguments[i],"$$",Args[i+1]);
//            i++;
//            Obj->Arguments[i] = strtok(NULL, LSH_TOK_DELIM);
        }
        else{
            Args[i+1] = (Obj->Arguments[i]);
        }
    }
    
    Args[i+1] = NULL;
}




/*********************************************************************
 *Name: Redirects
 *Function:give a redirects for input and output
 *********************************************************************/
void Redirects(struct InputObj* Obj)
{
    int InputFileDescriptor = STDIN_FILENO;
    int OutputFileDescriptor = STDOUT_FILENO;
    
    if(Obj->InputFile[0] != '\0'){ // check if inputs are active.
        //printf("INPUT: %s\n",Obj->InputFile);
        InputFileDescriptor = open(Obj->InputFile,O_RDONLY); // open file.
        
        if(InputFileDescriptor < 0){ // if not found exit.
            printf("badfile: no such file or directory.\n");
            exit(1);
        }
        dup2(InputFileDescriptor,0); // change input redirection.
        close(InputFileDescriptor); // close file.
    }
    if(Obj->OutputFile[0] != '\0'){ // check if outputs actve.
        //printf("OUTPUT: %s\n",Obj->OutputFile);
        // Can also use creat() func
        OutputFileDescriptor = open(Obj->OutputFile,O_WRONLY | O_CREAT | O_TRUNC,0644); // create new file or edit.
        
        if(OutputFileDescriptor < 0){ // check for error.
            printf("Error opening or creating file.");
            exit(1);
        }
        
        dup2(OutputFileDescriptor,1);//change output directions.
        close(OutputFileDescriptor);
    }
}


/*********************************************************************
 *Name:RunCommand
 *Function:created a children
 *********************************************************************/
void RunCommand(struct InputObj* Obj)
{
    pid_t pid = fork();
    char *ArgList[1024];
    int ProcessStatus;
    
    switch(pid)
    {
        case -1: //Error
            printf("Wwrong in fork.\n");
            exit(1);
            break;
            
        case 0: //Child
            Redirects(Obj);
            
            InitArgList(Obj,ArgList);
            execvp(Obj->Command, ArgList); // run command.
            
            printf("badfile: No such file or directory.\n");
            exit(1);
            break;
            
        default: // Parent
            if(Obj->Background == true && OnlyFore == false){ //setup bg or non bg.
                AddPid(pid);
                printf("Background Pid is %d\n",TopPid());
            }
            else{
                
                waitpid(pid,&ProcessStatus,0); // hang the shell is bg inactive.
                 GroundDataLast = ProcessStatus;
                //printf("parent(%d) waited for child process(%d)\n",getpid() ,pid);
            }
            break;
    }
}


/*********************************************************************
 *Name: StopSignalSign
 *Function: for ^Z command
 *Element:sig,output
 *********************************************************************/
void StopSignalSign(int sig)
{
    if(OnlyFore != false){
        char* output = "\nExiting foreground-only mode\n"; // exit Fg mode.
        write(STDOUT_FILENO, output, 31);
        OnlyFore = false; // change gloabl.
    }
    else{
      
        char* output = ("\nEntering foreground-only mode (& is now ignored)\n"); // enable Fg mode.
        write(STDOUT_FILENO, output, 50);
        OnlyFore = true;
    }
}


/// DESC: signal handler for a child process ending.

/*********************************************************************
 *Name: ChildSignalSign
 *Function:  track down the death of a child process
                without making the parent process wait until the child process got killed
 *Element:ChildPid,
 *Cite from:http://stackoverflow.com/questions/2377811/tracking-the-death-of-a-child-process
 *********************************************************************/
void  ChildSignalSign(int sig)
{
    
    pid_t ChildPid;
    int ChildState;
    int i;
    

    for(i = 0;i < BackStack.NumBackP + 1;i++){
        ChildPid = waitpid(BackStack.BackgroundP[i],&ChildState,WNOHANG);
        
        if((ChildState == 0 || ChildState == 1) && ChildPid != 0 ){ // if  child process exited or have some error in exited
            fprintf(stdout,"\nBackground pid %d is done: exit value %d\n",ChildPid,ChildState);
            RemoveEarly(ChildPid);
       
        }
        else if(ChildPid != 0){
            fprintf(stdout,"\nBackground pid %d is done: terminated by signal %d\n", ChildPid, ChildState);
            RemoveEarly(ChildPid);
       
        }
    }
    
}


/*********************************************************************
 *Name: TermSignalSign
 *Function: print signal
 *Element: signa
 *********************************************************************/
void TermSignalSign(int signa)
{
    printf("\n terminated by signal %d\n",signa);
}


/*********************************************************************
 *Name:FreeAndClearInputObj
 *Function:free
 *********************************************************************/
void FreeAndClearInputObj(struct InputObj* Obj)
{
    //int i;
    
    Obj->Background = false; // reset bg
    //clear all fields.
    memset(Obj->InputFile,'\0',sizeof(Obj->InputFile));
    memset(Obj->OutputFile,'\0',sizeof(Obj->OutputFile));
    memset(Obj->Command,'\0',sizeof(Obj->Command));
    
    // for(i = 0; i < Obj->NumArgs;i++){
    //     memset(Obj->Arguments[i],'\0',sizeof(Obj->Arguments[i]) );
    // }
    
    free(Obj);
}

/*********************************************************************
 *Name: KillProcesses
 *Function: exit
 *********************************************************************/
void KillProcesses()
{
    int i;
    for(i = 0;i < BackStack.NumBackP + 1;i++){
        kill(BackStack.BackgroundP[i], SIGINT); // interrupt all bg pids.
    }
}



/*********************************************************************
 *Name: CheckMode
 *Function: if return stop then change mode
 *********************************************************************/
void CheckMode()
{
    if(WTERMSIG(GroundDataLast) == 11 && OnlyFore == true){ // if signal is stop and fg true switch fg moce.
        printf("\nExiting foreground-only mode\n");
        OnlyFore = false;
    }
    else if(WTERMSIG(GroundDataLast) == 11 && OnlyFore == false){ // if signal is stop and fg false switch fg mode.
        printf("\nEntering foreground-only mode (& is now ignored)\n");
        OnlyFore = true;
    }
}

/*********************************************************************
 *Name: Replaceword
 *Function: replace a string with another
 * Cite from:https://www.geeksforgeeks.org/c-program-replace-word-text-another-given-word/
 *********************************************************************/
//char *ReplaceWord(const char *s, const char *oldW,const char *newW)
//{
//    char *result;
//    int i, cnt = 0;
//    int newWlen = strlen(newW);
//    int oldWlen = strlen(oldW);
//
//    // Counting the number of times old word
//    // occur in the string
//    for (i = 0; s[i] != '\0'; i++)
//    {
//        if (strstr(&s[i], oldW) == &s[i])
//        {
//            cnt++;
//
//            // Jumping to index after the old word.
//            i += oldWlen - 1;
//        }
//    }
//
//    // Making new string of enough length
//    result = (char *)malloc(i + cnt * (newWlen - oldWlen) + 1);
//
//    i = 0;
//    while (*s)
//    {
//        // compare the substring with the result
//        if (strstr(s, oldW) == s)
//        {
//            strcpy(&result[i], newW);
//            i += newWlen;
//            s += oldWlen;
//        }
//        else
//            result[i++] = *s++;
//    }
//
//    result[i] = '\0';
//    return result;
//}
//

/*********************************************************************
 *Name:Run
 *Function: Run shell
 *Element:
 *********************************************************************/
void Run()
{
    char InputBuffer[1024];
    struct InputObj *Obj;
    int ForegroundState;
   // char *r;
    
    struct sigaction StopSignal;
    StopSignal.sa_handler = StopSignalSign;
    StopSignal.sa_flags = 0;
    
    struct sigaction TermSignal;
    TermSignal.sa_handler = TermSignalSign;
    StopSignal.sa_flags = 0;
    
    struct sigaction ChildSignal;
    ChildSignal.sa_handler = ChildSignalSign;
    StopSignal.sa_flags = 0;
    //end init signals
    
    do
    {
        //reseting signal handlers.
        sigaction(SIGTSTP,&StopSignal, NULL);
        sigaction(SIGINT,&TermSignal, NULL);
        sigaction(SIGCHLD,&ChildSignal, NULL);
        
   
        CheckMode();
        
       
        fflush(stdout);//clearing stdout
        fflush(stdin);//clearing stdin
        
        printf(": ");
        memset(InputBuffer,'\0',sizeof(InputBuffer));
        fgets(InputBuffer,sizeof(InputBuffer),stdin); // get command line.
//        if (strstr(InputBuffer,"$$")!=NULL){
//                pid_t p = getpid();
//                char c [64];
//                sprintf(InputBuffer, "%d", p);
//                r = replaceWord(InputBuffer,"$$",);
//        }

   
        
        fflush(stdout);
        fflush(stdin);
        
        if(strncmp(InputBuffer,"exit",4) == 0){ // exit shell.
            
            KillProcesses();
            exit(0);
        }
        else if(strncmp(InputBuffer, "#",1) == 0){ // comment.
            continue;
        }
        else if(strncmp(InputBuffer,"cd", 2) == 0){ // change directory
       
            DirectChageInHom(InputBuffer);
        }
        else if(strncmp(InputBuffer,"status",6) == 0){ // check last fg command status.
            if(WEXITSTATUS( GroundDataLast)){
                ForegroundState = WEXITSTATUS(GroundDataLast); // check if extied.
            }
            else{
                ForegroundState = WTERMSIG( GroundDataLast); // check if terminated by signal.
            }
            printf("exit value %d\n",ForegroundState);
        }
        else{
            if(InputBuffer != NULL && strcmp(InputBuffer,"") != 0){
               
                Obj = malloc(1 * sizeof(struct InputObj));
                ParseInput(InputBuffer,Obj); // parse command line.
                RunCommand(Obj); //
                
                FreeAndClearInputObj(Obj);
            }
            else{
                continue;
            }
        }
    }
    while(true);
}

/*********************************************************************
            *main
*********************************************************************/
int main(void)
{
    InitailPid();
    Run();
    return 0;
}

