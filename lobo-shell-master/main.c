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
bool checkForRedirection(char **line_words);
bool checkForQuotes(char **line_words);
char **quotingParse(char **line_words);
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
    char** quotedCommand;
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
        bool existsQuote = false;
        int i;
        //num_strings is number of strings in entire command minus the number of pipes in our entire cmd line
        int num_strings = num_words - pipeCounter;
        
        //Check for no pipes
        if(pipeCounter == 0)
        {
            
            if(pid = fork() == 0)
            {
                existsQuote = checkForQuotes(line_words);
                if(existsQuote)
                {
                    quotedCommand = quotingParse(line_words);
                    // printf("%s, ", quotedCommand[1]);
                    execvp(quotedCommand[0], quotedCommand);
                    return 0;
                } 

                redirection = checkForRedirection(line_words);
                
                if(redirection)
                {
                    //stores command before redirection operator
                    arrForRedirection = wordsBeforeRedirection(line_words);
                    //Does redirection file description rewiring
                    doRedirection(arrForRedirection, line_words);
                    execvp(arrForRedirection[0], arrForRedirection);
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

                //Count words between pipes through each iteration
                wordCount = countWordsBetweenPipes(line_words, indexesToStart, i);

                //Create the command to execute between each pipe on each iteration
                arrForChild = createTempArray(line_words, wordCount, indexesToStart, i);
                redirection = false;
                redirection = checkForRedirection(arrForChild);

                if(pid = fork() == 0)
                {
                    if(i == 0)
                    {
                        if(redirection)
                        {
                            arrForRedirection = wordsBeforeRedirection(arrForChild);
                            doRedirection(arrForChild, line_words);
                        }
                            
                        dup2(pfdN[i][1], 1);
                        
                    }
                    else if(i == pipeCounter)
                    {
                        if(redirection)
                        {
                            arrForRedirection = wordsBeforeRedirection(arrForChild);
                            doRedirection(arrForChild, line_words);
                        }
                            
                    
                        dup2(pfdN[i - 1][0], 0);
                        
                    }
                    else if((i != 0) && i < pipeCounter)
                    {
                        if(redirection)
                        {
                            arrForRedirection = wordsBeforeRedirection(arrForChild);
                            doRedirection(arrForChild, line_words);
                        }
                        dup2(pfdN[i - 1][0], 0);
                        dup2(pfdN[i][1], 1);
                        
                    }
                    if(redirection)
                    {
                        closePfd(pfdN, pipeCounter);
                        execvp(arrForRedirection[0], arrForRedirection);
                        return 0;
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
    free(arrForRedirection);
    free(arrForChild);
    free(quotedCommand);
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

bool checkForRedirection(char **line_words)
{
    
    for(int i = 0; line_words[i] != NULL; i++)
    {
        if(strcmp(line_words[i], ">") == 0
        || strcmp(line_words[i], "<") == 0
        || strcmp(line_words[i], ">>") == 0)
        {
            return true;
        }
    }
    return false;
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
    
}

bool checkForQuotes(char **line_words)
{
    for(int i = 0; line_words[i] != NULL; i++)
    {
        for(int j = 0; line_words[i][j] != '\0'; j++)
        {
            if(line_words[i][j] == '"')
                return true;
        }
    }
    return false;
}

char **quotingParse(char **line_words)
{
    int i;
    int j;
    int wordCount = 0;
    int quotecount = 0;
    int customIterator = 0;
    char** newQuoteParseCommand;
    char tempString[MAX_LINE_CHARS] = "";
    
    //Before concatenating, find how much space we need to dynamically allocate for new command string with 
    //quote escaping
    for(i = 0; line_words[i] != NULL; i++)
    {
        for(j = 0; line_words[i][j] != '\0'; j++)
        {
            if(line_words[i][j] == '"')
            {
                quotecount++;
                break;
            }
            
        }
        if(quotecount == 0)
            wordCount++;
        else if(quotecount == 2)
            wordCount++;
    }

    
    //Allocate proper memory size
    newQuoteParseCommand = malloc(sizeof(char**) * wordCount);
    for(i = 0; i < wordCount; i++)
        newQuoteParseCommand[i] = malloc(sizeof(char*) * 1024);

    quotecount = 0;

    // //Concatenate without quotes
    for(i = 0; line_words[i] != NULL; i++)
    {
        for(j = 0; line_words[i][j] != '\0'; j++)
        {
            //If we find a quote, left shift all chars to essentially erase it.
            if(line_words[i][j] == '"')
            {
                quotecount++;
                while(line_words[i][j] != '\0')
                {   
                    line_words[i][j] = line_words[i][j + 1];
                    j++;
                }
                
            }
            
        }
        

    //     //If we haven't found quote, then copy linewords string to new adjusted command string to be returned
        if(quotecount == 0)
            strcpy(newQuoteParseCommand[i], line_words[i]);
        
        
        else if(quotecount == 1)
        {
            //printf("%s, ", line_words[i]);
            strcat(tempString, line_words[i]);
            
            strcat(tempString, " ");
            printf("%s,", tempString);
        }
        else
        {
            strcat(tempString, line_words[i]);
            customIterator++;
            strcpy(newQuoteParseCommand[customIterator], tempString);
        }
    
    }
    
    printf("%s, ", tempString);
    newQuoteParseCommand[customIterator + 1] = 0x0;
    return newQuoteParseCommand;
}
