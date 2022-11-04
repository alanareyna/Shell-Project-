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



//Functions prototypes
int checkForPipes(char** line_words);
void closePfd(int pfd[][2], int pipeCounter);
char **createTempArray(char **line_words, int wordCount, int *indexesToStart, int index);
int *whichIdxToStart(char **line_words, int pipeCounter);
int countWordsBetweenPipes(char **line_words, int *indexesToStart, int index);
char** wordsBeforeRedirection(char **line_words);
void doRedirection(char **arrForRedirection, char **line_words);
int main()
{
    // Buffer for reading one line of input
    char line[MAX_LINE_CHARS];
    // holds separated words based on whitespace
    char* line_words[MAX_LINE_WORDS + 1];
    
    //Fork Variable
    pid_t pid;

    int pipeCounter = 0;

    //Array of strings used to hold commands separated by pipes
    char** arrForChild;
    char** arrForRedirection;
    //Counts # of words between pipes
    int wordCount;
    


    while( fgets(line, MAX_LINE_CHARS, stdin) ) 
    {
        //line is char Array
        //line_words is array of strings
        int num_words = split_cmd_line(line, line_words);

        //Number of pipes in our entire command line
        pipeCounter = checkForPipes(line_words);
        bool redirection = false;

        int i;
        //num_strings is number of strings in entire command minus the number of pipes in our entire cmd line
        int num_strings = num_words - pipeCounter;
        
        //Check for no pipes
        if(pipeCounter == 0)
        {
            if(pid = fork() == 0)
            {
                for(i = 0; line_words[i] != NULL; i++)
                {
                    if(strcmp(line_words[i], ">") == 0
                    || strcmp(line_words[i], "<") == 0
                    || strcmp(line_words[i], ">>") == 0)
                    {
                        redirection = true;
                        break;
                    }
                    
                }
                if(redirection)
                {
                    //stores command before redirection operator
                    arrForRedirection = wordsBeforeRedirection(line_words);
                    //Does redirection file description rewiring
                    doRedirection(arrForRedirection, line_words);
                }
                
                
                else
                {
                    execvp(line_words[0], line_words);
                    return 0;
                }
                
            }
            while(wait(NULL) != -1){
            };
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
                arrForChild = createTempArray(line_words, wordCount, indexesToStart, i);

                if(pid = fork() == 0)
                {
                    if(i == 0)
                    {
                        dup2(pfdN[i][1], 1);
                        
                    }
                    else if(i == pipeCounter)
                    {
                        dup2(pfdN[i - 1][0], 0);
                        
                    }
                    else if((i != 0) && i < pipeCounter)
                    {
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

char** wordsBeforeRedirection(char **line_words)
{
    int count = 0;
    int i = 0;
    while(line_words[i] != NULL)
    {
        if(strcmp(line_words[i], ">") == 0
        || strcmp(line_words[i], "<") == 0
        || strcmp(line_words[i], ">>") == 0)
            break;
        count++;
        i++;
    }
    char **temp = malloc(sizeof(char**) * count);
    for(int x = 0; x < count; x++)
        temp[x] = malloc(sizeof(char*) * 1024);
    
    i = 0;
    for(; i < count; i++)
        strcpy(temp[i], line_words[i]);
    temp[i] = 0x0;
    return temp;


}

void doRedirection(char **arrForRedirection, char **line_words)
{
    int in;
    int outOrAppend;
    for(int i = 0; line_words[i] != NULL; i++)
    {
        //input redirection
        if(strcmp(line_words[i], "<") == 0)
        {
            in = open(line_words[i + 1], O_RDONLY, 0);
            dup2(in, 0);
            close(in);
            
        }
        if(strcmp(line_words[i], ">") == 0)
        {
            outOrAppend = open(line_words[i + 1], O_CREAT | O_WRONLY, 0664);
            dup2(outOrAppend, 1);
            close(outOrAppend);
            
        }
        else if(strcmp(line_words[i], ">>") == 0)
        {
            outOrAppend = open(line_words[i + 1], O_APPEND | O_CREAT | O_WRONLY, 0664);
            dup2(outOrAppend, 1);
            close(outOrAppend);
        }

    }
    execvp(arrForRedirection[0], arrForRedirection);
}
