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
char **createTempArray(char **line_words, int wordCount, int *indexesToStart, int index);
int *whichIdxToStart(char **line_words, int pipeCounter);
int countWordsBetweenPipes(char **line_words, int *indexesToStart, int index);
int main()
{
    // Buffer for reading one line of input
    char line[MAX_LINE_CHARS];
    // holds separated words based on whitespace
    char* line_words[MAX_LINE_WORDS + 1];
    // char*** commandsArray;
    pid_t pid;
    int pipeCounter = 0;
    char** arrForChild;
    int wordCount;
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
            if(pid = fork() == 0)
            {
                execvp(line_words[0], line_words);
                return 0;
                
            }
        }
        else if(pipeCounter > 0)
        {
            int pfdN[pipeCounter][2];
            for(int j = 0; j < pipeCounter; j++)
                pipe(pfdN[j]);

            int *indexesToStart = whichIdxToStart(line_words, pipeCounter);
        
            for(int i = 0; i < pipeCounter + 1; i++)
            {

                wordCount = countWordsBetweenPipes(line_words, indexesToStart, i);
                // printf("%d, ", wordCount);
                // exit(1);
                arrForChild = createTempArray(line_words, wordCount, indexesToStart, i);
                // for(int x = 0; arrForChild[x] != NULL; x++)
                //     printf("%s, ", arrForChild[x]);
                // printf("\n");
                // free(arrForChild);
                // //fork child process
                if(pid = fork() == 0)
                {
                    if(i == 0)
                    {
                        //printf("At beginning\n");
                        dup2(pfdN[i][1], 1);
                        
                    }
                    else if(i == pipeCounter)
                    {
                        //printf("At end\n");
                        dup2(pfdN[i - 1][0], 0);
                        
                    }
                    else if((i != 0) && i < pipeCounter)
                    {
                        //printf("%d, ", i);
                        //printf("At middle command\n");
                        dup2(pfdN[i - 1][0], 0);
                        dup2(pfdN[i][1], 1);
                        
                    }
                    closePfd(pfdN, pipeCounter);
                    execvp(arrForChild[0], arrForChild);
                    return 0;
                   
                }
            }
            closePfd(pfdN, pipeCounter);
            while(wait(NULL) != -1){
            };
            
        }
       
    };
    return 0;

}

    




int countWordsBetweenPipes(char **line_words, int *indexesToStart, int index)
{
    int wordCount = 0;
    
    for(int i = indexesToStart[index]; line_words[i] != NULL ; i++)
    {
        if(line_words[i] == NULL || strcmp(line_words[i], "|") == 0)
            break;
        wordCount++;
    } 

    return wordCount;
}

int *whichIdxToStart(char **line_words, int pipeCounter)
{
    int j = 1;
    int *tempArray = malloc(sizeof(int) * pipeCounter + 1);
    tempArray[0] = 0;
    for(int i = 0; line_words[i] != NULL; i++)
    {
        if(strcmp(line_words[i], "|") == 0)
        {
            tempArray[j] = i + 1;
            j++;
        }
    }
    return tempArray;
}
char **createTempArray(char **line_words, int wordCount, int *indexesToStart, int index)
{
    int i = 0;
    int j = indexesToStart[index];

    char **temp = malloc(sizeof(char**) * wordCount + 1);
    for(int words = 0; words < wordCount; words++)
        temp[words] = malloc(sizeof(char*) *  1024);
    
    for(; i < wordCount; i++, j++)
        // printf("%s, ", line_words[j]);
        strcpy(temp[i], line_words[j]);
    
    //Null terminate the strings or crash
    temp[i] = 0x0;
    
    return temp;
        
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
        if(strcmp(line_words[i], "|") == 0)
            returnPipes++;
    }
    return returnPipes;
}

