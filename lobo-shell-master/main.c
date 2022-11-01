#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include "constants.h"
#include "parsetools.h"
#include <stdbool.h>

// struct{
//     int isBeginning;
// }beginning = {1};

//Functions prototypes
int checkForPipes(char** line_words);
char*** storeAllCommands(char** line_words, int pipe_counter);
void storeInto(char** dest, char** src, int indexToStartAt, int countWords);
void closePfd(int pfd[][2], int pipeCounter);
int main()
{
    // Buffer for reading one line of input
    char line[MAX_LINE_CHARS];
    // holds separated words based on whitespace
    char* line_words[MAX_LINE_WORDS + 1];
    char*** commandsArray;
    pid_t pid;
    int pipeCounter = 0;

    // or some other input error occurs
    while( fgets(line, MAX_LINE_CHARS, stdin) ) 
    {
        //line is char Array
        //line_words is array of strings
        int num_words = split_cmd_line(line, line_words);

        //Number of pipes in our entire command line
        pipeCounter = checkForPipes(line_words);
        
        //num_strings is number of strings in entire command minus the number of pipes in our entire cmd line
        int num_strings = num_words - pipeCounter;
        
        //Check for no pipes
        if(pipeCounter == 0)
        {
            if(pid == fork() == 0)
            {
                execvp(line_words[0], line_words);
                while(wait(NULL) != 1);
            }
        }
        else if(pipeCounter > 0)
        {
            int pfdN[pipeCounter][2];
            for(int j = 0; j < pipeCounter; j++)
                pipe(pfdN[j]);
            //Store all commands in 3d array
            commandsArray = storeAllCommands(line_words, pipeCounter);
            for(int i = 0; i < pipeCounter + 1; i++)
            {
                //fork child process
                if(pid = fork() == 0)
                {
                    if(i == 0)
                    {
                        dup2(pfdN[i][1], 1);
                    }
                    if(i == pipeCounter)
                    {
                        dup2(pfdN[i - 1][0], 0);
                    }
                    if((i != 0) && i < pipeCounter)
                    {
                        dup2(pfdN[i][1], 1);
                        dup2(pfdN[i - 1][0], 0);
                    }
                    closePfd(pfdN, pipeCounter);
                    execvp(commandsArray[i][0], commandsArray[i]);
                    return 0;
                }
            }
            closePfd(pfdN, pipeCounter);
            while(wait(NULL) != -1){
            };
        }

        
        //Switch passed # of pipes and logic will be taken care of for each test
        // switch(pipeCounter)
        // {
            
        //     case 0:
        //         if(pid = fork() == 0)
        //         {
        //             execvp(line_words[0], line_words);
        //             while(wait(NULL) != -1);
                    
        //         }
        //         break;
        //     case 1:
        //         //printf("In case 1, pipecounter = %d: ", pipeCounter);
        //         int pfd[2];
        //         pipe(pfd);
                
        //         for(int i = 0; i < (pipeCounter + 1); i++)
        //         {
                    
        //             //Parent needs to control when at beginning and end process
        //             beginning.isBeginning = (i == 0) ? 1 : 0;
        //             pid = fork();
        //             if(pid == -1)
        //             {
        //                 printf("Error creating child process\n");
        //                 exit(1);
        //             }
        //             if(pid == 0)
        //             {
        //                 //Child fork execution
        //                 switch(beginning.isBeginning)
        //                 {
        //                     //Case 0 is end process logic
        //                     case 0:
        //                         dup2(pfd[0], 0);
        //                         if(close(pfd[0]) == -1 || close(pfd[1]) == -1)
        //                             strerror(errno);
        //                         execvp(commandsArray[1][0], commandsArray[1]);
        //                         return 0;
        //                     //Case 1 is beginning process logic
        //                     case 1:
        //                         dup2(pfd[1], 1);
        //                         if(close(pfd[1]) == -1 || close(pfd[0]) == -1)
        //                             strerror(errno);
        //                         execvp(commandsArray[0][0], commandsArray[0]);
        //                         return 0;
                         
        //                 };
        //             }
                    
        //         }
        //         //reap children and close fds
        //         if(close(pfd[0]) == -1 || close(pfd[1]) == -1)
        //         {
        //             printf("Parent had error closing stdin and stdout");
        //             exit(1);
        //         }
                
        //         while(wait(NULL) != -1){
        //         };
        //         break;
            
        //     default:
        //         //create ptr to pfd's of size 2
        //         int pfdN[pipeCounter][2];
        //         int i;
                
        //         //Initialize all pfd ptrs to their respective pipes
        //         for(i = 0; i < pipeCounter; i++)
        //         {
        //             pipe(pfdN[i]);
        //         }
                
                
        //         //Main loop for dealing with fd ptrs
        //         for(i = 0; i < (pipeCounter + 1); i++)
        //         {
                    
        //             // if(i == 0)
        //             //     //Beginning
        //             //     beginning.isBeginning = 1;
        //             // else if(i > 0 && i < pipeCounter)
        //             //     //Middle
        //             //     beginning.isBeginning = 2;
        //             // else
        //             //     //End
        //             //     beginning.isBeginning = 0;
                    
        //             //Call Fork
        //             pid = fork();
                    
        //             if(pid == -1)
        //             {
        //                 printf("Error creating child process\n");
        //                 exit(1);
        //             }
        //             //Check for child process
        //             if(pid == 0)
        //             {
        //                 if(i != pipeCounter)
        //                     dup2(pfdN[i][1], 1);
        //                 if(i != 0)
        //                     dup2(pfdN[i - 1][0], 0);
        //                 closePfd(pfdN, pipeCounter);
        //                 execvp(commandsArray[i][0], commandsArray[i]);
        //                 return 0;
        //                 // switch(beginning.isBeginning)
        //                 // {
                            
        //                 //     case 0:
        //                 //         // printf("Executing End Command\n");
        //                 //         // printf("%d", i);
        //                 //         // printf("%s", commandsArray[i][0]);
        //                 //         //End child should pass in i = 2 => pfdN[1][0](2nd pipe, read end
        //                 //         dup2(pfdN[i - 1][0], 0);
        //                 //         closePfd(pfdN, pipeCounter);
        //                 //         execvp(commandsArray[i][0], commandsArray[i]);
        //                 //         return 0;
        //                 //     //Case 1: Beginning Child should pass in i = 0  => pfdN[0][1](First pipe write end)
        //                 //     case 1:
        //                 //         // printf("Executing Beginning Command\n");
        //                 //         // printf("%d\n", i);
        //                 //         // printf("%s", commandsArray[i][0]);
        //                 //         dup2(pfdN[i][1], 1);
        //                 //         closePfd(pfdN, pipeCounter);
        //                 //         execvp(commandsArray[i][0], commandsArray[i]);
        //                 //         return 0;
        //                 //     case 2:
        //                 //         // printf("Executing Mid Command\n");
        //                 //         // printf("%d\n", i);
        //                 //         // printf("%s", commandsArray[i][0]);
        //                 //         //Middle child should pass in i = 1 => pfdN[0][0](First pipe read end) pfdN[1][1](2nd pip Write end)
        //                 //         //For example, every time we have a Child process in the middle of two pipes, it should look backwards to the read end
        //                 //         //of the previous pipe and connect its 0 index, and also look forward to the next pipe and connect its write end
        //                 //         dup2(pfdN[i - 1][0], 0);
        //                 //         dup2(pfdN[i][1], 1);
        //                 //         closePfd(pfdN, pipeCounter);
        //                 //         execvp(commandsArray[i][0], commandsArray[i]);
        //                 //         return 0;
        //                 // };
                        
        //             }

                    
                  
        //         }
        //         //Close parent's pipe pointers and reap children
        //         //for(int j = 0; j < pipeCounter; j++)
        //         // {
        //         //     if(close(pfdN[j][0]) == -1 || close(pfdN[j][1]) == -1)
        //         //     {
        //         //         printf("Error closing pfd2 from beginning process\n");
        //         //         exit(1);
        //         //     }
                        
        //         // }
        //         closePfd(pfdN, pipeCounter);
        //         while(wait(NULL) != -1){
        //         };
        //         break;
        // };

    }

    return 0;
}



void closePfd(int pfd[][2], int pipeCounter)
{
    
    for(int i = 0; i < pipeCounter; i++)
    {
        if(close(pfd[i][0]) == -1 || close(pfd[i][1]) == -1)
        {
            printf("Error closing pfd2 from beginning process\n");
            exit(1);
        }
            
    }
}
//Counting pipes function
int checkForPipes(char** line_words)
{
    int returnPipes = 0;
    for(int i = 0; line_words[i] != NULL; i++)
    {
        if((strcmp(line_words[i], "|") == 0))
            returnPipes++;
    }
    return returnPipes;
}

// [1] = addr of string array [2] = string itself [3] = char inside of string
char*** storeAllCommands(char** line_words, int pipe_counter)
{
    //Allocate memory for the number of arrays of strings we are pointing to
    //For example, if we have one pipe in the entire command line, we will have two full commands(pipeCounter + 1)

    //New strategy: Dynamically allocate new string arrays and have returningArray point to them.
    //After execd in main, free (all of the indexes to returningArray? or just the base address? not sure which needs to be done)
    char*** returningArray = malloc(sizeof(char***) * (pipe_counter + 1));
    int returnIndex = 0;
    int countWords = 0;
    int startCounter = 0;
    bool start = true;
    int idxToStart = -1;
    for(int i = 0; line_words[i] != NULL; i++)
    {
        //need to check for no pipes and store again
        if((strcmp(line_words[i], "|") == 0) || line_words[i + 1] == NULL)
        {
            //We are going to store the whole string up until a pipe in this newly dynamically
            //allocated ptr to strings. Then after this, point returningArray at a specified index to the
            //base address of this newly allocated array.
            if(start)
            {
                idxToStart = 0;
                start = false;
            }  
            
            char** tempToBeAssigned = malloc(sizeof(char**) * countWords);
            for(int i = 0; i < countWords; i++)
            {
                tempToBeAssigned[i] = malloc(sizeof(char*) * 1024);
            }
            storeInto(tempToBeAssigned, line_words, idxToStart, countWords);
            idxToStart = i + 1;
            returningArray[returnIndex] = tempToBeAssigned;
            returnIndex++;
            countWords = 0;

            
        }
        countWords++;
       
    }
    return returningArray;
    
}

void storeInto(char** dest, char** src, int indexToStartAt, int countWords)
{   

    for(int i = 0, j = indexToStartAt; i < countWords ; i++, j++)
    {
        strcpy(dest[i], src[j]);
    }
    
}


