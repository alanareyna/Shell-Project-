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
/*Andrew Notes:

NOTE: It seems that the total number of fork() calls depends on our number of
pipes(|) + 1. So we will need an incrementation of a pipe counter in terms of how
many times it shows up in our line_words.

Note: We might need to look into dynamic memory allocation for allocating enough memory for pidt array
to be passed into pipe() function. This will be dependant on our actual pipe (|) counter that we will be updating 
as we parse through our linwe_words.

Note: Might need to separate line_words into separate array of strings (char**) that begin at 
[0] index so that we can pass the execvp(assuming we have to use execvp in the first place)

For 1 command Tests:
    - We keep track of the number of strings that are going to be 
      stored from our split_cd_line() function. If we find no pipe (|)
      characters, i.e our pipe counter is equal to zero, that implies
      that we should only have one command in our array. This should fork() once.
      We can probably use execlp for one command.

For 1 pipe:
    - We can parse through the function and keep a counter of how many pipe (|) 
      characters we have in line_words which is a single character array. If we only
      have one pipe, that means we should have two valid commands on each side of
      that singular pipe. Write end of first Fork() FD should go to write end of singular
      pipe, Read end should go to stdin(default), and err should go to screen(default). Write end
      of second Fork() goes to stdout(default), read end of FD at read end of singular pipe
      and ERR fd remains in default screen. (This is all variable to change based on our actual commands
      and what they can possibly be).

For more than 1 pipe:
     - Number of forks is going to be pipe(|) countter + 1. Logic of first and last Fork() FD's is  
       similar to our logic in "1 command tests", everything in between will be similar to the logic
       in "For 1 pipe". 
       

*/

int checkForPipes(char** line_words);
<<<<<<< Updated upstream
=======
char*** storeAllCommands(char** line_words, int pipe_counter);
void storeInto(char** dest, char** src, int indexToStartAt, int countWords);
>>>>>>> Stashed changes
int main()
{
    // Buffer for reading one line of input
    char line[MAX_LINE_CHARS];
    // holds separated words based on whitespace
    char* line_words[MAX_LINE_WORDS + 1];

    int pipeCounter = 0;

    // or some other input error occurs
    while( fgets(line, MAX_LINE_CHARS, stdin) ) {

        //line is char Array
        //line_words is array of strings
        int num_words = split_cmd_line(line, line_words);
        //Number of pipes in our entire command line
        pipeCounter = checkForPipes(line_words);
        switch(pipeCounter){
            case 0:
                pid_t pid;
                if(pid = fork() == 0)
                {
                    execvp(line_words[0], line_words);
                    while(wait(NULL) != -1);
                }
<<<<<<< Updated upstream
            
    }
=======
            case 1:
                // pid_t pid1;
                // pid_t pid2;
                // int pfd1[2];
                // int pfd2[2];
                beegYoshi = storeAllCommands(line_words, pipeCounter);
                
                for(int i = 0; i < 2; i++)
                {
                    for(int j = 0; j < (pipeCounter + 1); j++)
                    {
                        
                        printf("%s, ", beegYoshi[i][j]);
                    }
                }
                


        }
>>>>>>> Stashed changes

    }

    
    
    return 0;
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
<<<<<<< Updated upstream
=======

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
    //printf("%d ,", indexToStartAt);
    
    // //SEG FAULT RIGHT HERE THERE BE A SEG FAULT RIGHT HERE AHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
    for(int i = 0, j = indexToStartAt; i < countWords ; i++, j++)
    {
        //printf("%s, ", src[j]);
        strcpy(dest[i], src[j]);
    }
    
}
>>>>>>> Stashed changes
