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
void doRedirection(char **line_words);
bool checkForRedirection(char **line_words);
bool checkForQuotes(char **line_words);
char **quotingParse(char **line_words);
void syserror(const char *);

int main()
{
    //Buffer for reading one line of input
    char line[MAX_LINE_CHARS];

    //Holds separated words based on whitespace
    char* line_words[MAX_LINE_WORDS + 1];
    
    //Variable to keep track of child fork()
    pid_t pid;

    int pipeCounter = 0;

    //Array of strings used to hold Child fork command, redirection command
    //and double quote escape command
    char** arrForChild;
    char** arrForRedirection;
    char** quotedCommand;

    //Counts # of words between pipes
    int wordCount;
    


    while( fgets(line, MAX_LINE_CHARS, stdin) ) 
    {
        int num_words = split_cmd_line(line, line_words);

        //Number of pipes in our entire command line
        pipeCounter = checkForPipes(line_words);

        //Booleans to flag if we are detecting redirection or quote escaping
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
                //Sets boolean flag for quote character detection
                existsQuote = checkForQuotes(line_words);
                
                //Quote escape logic
                if(existsQuote)
                {
                    //Parse command to concatenate entire string delimited by double quotes
                    quotedCommand = quotingParse(line_words);
                    execvp(quotedCommand[0], quotedCommand);
                    syserror( "Could not exec" );
                    return 0;
                } 

                //Sets boolean flag for redirection operation
                redirection = checkForRedirection(line_words);
                if(redirection)
                {
                    //stores entire command before reaching redirection operator
                    arrForRedirection = wordsBeforeRedirection(line_words);

                    //Does redirection file descriptor rewiring
                    doRedirection(line_words);
                    execvp(arrForRedirection[0], arrForRedirection);
                    syserror( "Could not exec" );
                }
                //If no detection of redirection and quoting, execute singular no pipe command
                else
                {
                    execvp(line_words[0], line_words);
                    syserror( "Could not exec" );
                    return 0;
                }
                
            }
            else if(pid < 0)
            {
                syserror( "Fork failed" );
                break;
            }
            //Reap children
            while(wait(NULL) != -1){
            };
        }
        else if(pipeCounter > 0)
        {
            //Initialize 2D array of pfd variables in correspondence to the number of pipes in command
            int pfdN[pipeCounter][2];
            
            //Initialize pfd variables to their respective pipes in ascending order
            for(int j = 0; j < pipeCounter; j++)
                if(pipe(pfdN[j]) == -1)
                {
                    syserror( "Could not create a pipe" );
                }


            //Pointer to dynamically allocated integer array that stores which index we are starting our
            //command storage at. (0, pipe index + 1...etc)
            int *indexesToStart = whichIdxToStart(line_words, pipeCounter);
        
            for(int i = 0; i < pipeCounter + 1; i++)
            {
                
                //Count words between pipes through each iteration
                wordCount = countWordsBetweenPipes(line_words, indexesToStart, i);

                //Create the command to execute between each pipe on each iteration
                arrForChild = createTempArray(line_words, wordCount, indexesToStart, i);

                existsQuote = false;
                existsQuote = checkForQuotes(arrForChild);
                //We will check every iteration if our newly formed command between pipes includes
                //a redirection operator
                redirection = false;
                redirection = checkForRedirection(arrForChild);

                //Fork child process
                if(pid = fork() == 0)
                {
                    //First command before any pipes are reached
                    if(i == 0)
                    {
                        if(existsQuote)
                        {
                            
                            //Parse command to concatenate entire string delimited by double quotes
                            quotedCommand = quotingParse(arrForChild);
                            
                        } 
                        if(redirection)
                        {
                            //Store command we need to use for redirection execvp
                            arrForRedirection = wordsBeforeRedirection(arrForChild);

                            //Pass entire redirection command to adjust file descriptors
                            doRedirection(line_words);
                        }
                            
                        dup2(pfdN[i][1], 1);
                        
                    }
                    //End command 
                    else if(i == pipeCounter)
                    {
                         if(existsQuote)
                        {
                            //Parse command to concatenate entire string delimited by double quotes
                            quotedCommand = quotingParse(arrForChild);
                            
                        } 
                        if(redirection)
                        {
                            //Store command we need to use for redirection execvp
                            arrForRedirection = wordsBeforeRedirection(arrForChild);

                            //Pass entire redirection command to adjust file descriptors
                            doRedirection(line_words);
                        }
                            
                    
                        dup2(pfdN[i - 1][0], 0);
                        
                    }
                    //All commands that are not beginning and end of pipes
                    else if((i != 0) && i < pipeCounter)
                    {
                         if(existsQuote)
                        {
                            //Parse command to concatenate entire string delimited by double quotes
                            quotedCommand = quotingParse(arrForChild);
                            
                        } 
                        if(redirection)
                        {
                            //Store command we need to use for redirection execvp
                            arrForRedirection = wordsBeforeRedirection(arrForChild);

                            //Pass entire redirection command to adjust file descriptors
                            doRedirection(line_words);
                        }

                        dup2(pfdN[i - 1][0], 0);
                        dup2(pfdN[i][1], 1);
                        
                    }
                    //Execs arrForRedirection which previously will be holding the command excluding
                    //the redirection operator
                    if(existsQuote)
                    {
                        closePfd(pfdN, pipeCounter);
                        execvp(quotedCommand[0], quotedCommand);
                        syserror("Could not exec");
                        return 0;
                    }
                    if(redirection)
                    {
                        closePfd(pfdN, pipeCounter);
                        execvp(arrForRedirection[0], arrForRedirection);
                        syserror("Could not exec");
                        return 0;
                    }
                    //Execs command that aren't redirection commands
                    closePfd(pfdN, pipeCounter);
                    execvp(arrForChild[0], arrForChild);
                    syserror("Could not exec");
                    return 0;
                   
                }
                else if(pid < 0)
                {
                    syserror( "Fork failed" );
                    break;
                }
            }
            //Close parents FD
            closePfd(pfdN, pipeCounter);
            while(wait(NULL) != -1){
            };
            
        }

       
    };

    //Free all possible dynamically allocated memory
    free(arrForRedirection);
    free(arrForChild);
    free(quotedCommand);
    return 0;

}


/**
 * @brief 
 * Function to cound words between pipes excluding the pipe
 * @param line_words
 * Entire command line of strings after the user presses enter
 * @param indexesToStart 
 * Indicates where we will begin our counting from in line_words each 
 * time the function is called
 * @param index 
 * Used to dereference indexes to start which is an array that stores all pipe + 1 indexes in line_words
 * 
 * @return int
 * Returns word count between pipes to user
 */

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

/**
 * @brief 
 * Loops through entirety of line_words and will store index values in a dynamically
 * allocated integer array where we will start our index into line_words when counting
 * the number of words between each pipe. 
 * @param line_words 
 * Entire command line of strings after the user presses enter
 * @param pipeCounter 
 * How many pipes we have counted in the entirety of line_words
 * @return int* 
 * Returns dynamically allocated integer array that stores all indexes of where we will begin our
 * command parsing. This is so we can deal with excluding our pipes in line_words
 */

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

/**
 * @brief 
 * Creates an array of pointers to strings that will be used to hold the current command
 * we will be executing, excluding pipes
 * @param line_words 
 * Entire command line of strings after the user presses enter
 * @param wordCount 
 * Number of words between pipes
 * @param indexesToStart 
 * Array that holds the index we will be starting from when parsing and storing current command
 * @param index 
 * Uses 'i' from for loop in main that corresponds to which command in order from left to right
 * in the command we will be storing and executing
 * @return char** 
 * Returns dynamically allocated array of strings, which stores entire command to be executed
 * before file descriptor processing
 */

char **createTempArray(char **line_words, int wordCount, int *indexesToStart, int index)
{
    int i = 0;
    int j = indexesToStart[index];

    //Allocate properly sized array of strings
    char **temp = malloc(sizeof(char**) * wordCount + 1);
    for(int words = 0; words < wordCount; words++)
        temp[words] = malloc(sizeof(char*) *  1024);
    
    //Copy command excluding pipes into temporary array of strings
    for(; i < wordCount; i++, j++)
        strcpy(temp[i], line_words[j]);
    
    temp[i] = 0x0;
    return temp;
        
}

/**
 * @brief 
 * Function to handle closing all active file descriptors in children and parent process
 * @param pfd 
 * Pointer to pfd variables from caller in main
 * @param pipeCounter 
 * Used as an iterator to close all active file descriptors for pipes
 */

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

/**
 * @brief 
 * Counts the total number of pipes present in line_words
 * @param line_words 
 * Entire command line of strings after the user presses enter
 * @return int 
 * Returns number of pipes in line_words to caller
 */

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

/**
 * @brief 
 * Function that is used to count and store the number of words that come before a 
 * redirection operator inside of line_words
 * @param line_words 
 * Entire command line of strings after the user presses enter
 * @return char** 
 * Returns dynamically allocated array of strings that will hold the command that comes
 * before the redirection operator in line_words
 */

char** wordsBeforeRedirection(char **line_words)
{
    int count = 0;
    int i = 0;
    
    //While redireciton operator is not found, increment word count
    while(line_words[i] != NULL)
    {
        if(strcmp(line_words[i], ">") == 0
        || strcmp(line_words[i], "<") == 0
        || strcmp(line_words[i], ">>") == 0)
            break;
        count++;
        i++;
    }
    //Allocate properly sized array of strings to store command strings up until
    //redirection operator
    char **temp = malloc(sizeof(char**) * count);
    for(int x = 0; x < count; x++)
        temp[x] = malloc(sizeof(char*) * 1024);
    
    i = 0;
    //Store commands
    for(; i < count; i++)
        strcpy(temp[i], line_words[i]);

    temp[i] = 0x0;
    return temp;


}

/**
 * @brief 
 * Checks if line_words has a redirection operator present
 * @param line_words 
 * Entire command line of strings after the user presses enter
 * @return true 
 * Returns true if a redirection operator is found in line_words
 * @return false 
 * Returns false if a redirection operator is not found in line_words
 */

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

/**
 * @brief 
 * Function that handles the file descriptor processing when dealing with a command that 
 * uses redirection
 * @param line_words 
 * Entire command line of strings after the user presses enter
 */

void doRedirection(char **line_words)
{
    int in;
    int outOrAppend;

    for(int i = 0; line_words[i] != NULL; i++)
    {
        //Input redirection file descriptor logic
        if(strcmp(line_words[i], "<") == 0)
        {
            in = open(line_words[i + 1], O_RDONLY, 0);
            dup2(in, 0);
            close(in);
            
        }
        //Output redirection file descriptor logic
        if(strcmp(line_words[i], ">") == 0)
        {
            outOrAppend = open(line_words[i + 1], O_CREAT | O_WRONLY, 0664);
            dup2(outOrAppend, 1);
            close(outOrAppend);
            
        }
        //Append redirection file descriptor logic
        else if(strcmp(line_words[i], ">>") == 0)
        {
            outOrAppend = open(line_words[i + 1], O_APPEND | O_CREAT | O_WRONLY, 0664);
            dup2(outOrAppend, 1);
            close(outOrAppend);
        }

    }
    
}

/**
 * @brief 
 * Checks if line_words has a double quote (") present in entirety of command line string
 * @param line_words 
 * Entire command line of strings after the user presses enter
 * @return true 
 * Returns true if (") is found
 * @return false 
 * Returns false if (") is not found
 */

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

/**
 * @brief 
 * Parses through all of line_words and detects when we run into a double quote character (").
 * Once one is found, the current string that is being parsed through in line_words at some index 'i'
 * will have all of its characters left shifted to essentially delete the (") character while maintaining
 * proper string structure. A new temporary string (tempString) will be used to concatenate all separate strings
 * from line_words into itself until a second (") is found. Everything inbetween these double quotes will now 
 * be considered a singular string instead of multiple separate ones, and will be stored into a new corrected
 * command string during iteration.
 * This function will also handle escaped double quotes (\")
 * @param line_words 
 * Entire command line of strings after the user presses enter
 * @return char** 
 * Newly corrected array of strings to be returned to caller in main for quotation escape execution
 */

char **quotingParse(char **line_words)
{
    int i;
    int j;
    int wordCount = 0;
    int quoteCount = 0;
    int customIterator = 0;
    char** newQuoteParseCommand;
    char tempString[MAX_LINE_CHARS] = "";
    
    //Before concatenating, find how much space we need to dynamically allocate for new command string with 
    //quote escaping
    for(i = 0; line_words[i] != NULL; i++)
    {
        
        for(j = 0; line_words[i][j] != '\0'; j++)
        {
            if(line_words[i][j] == '"' && line_words[i][j-1] != '\\')
            {
                quoteCount++;
            }
            
        }
        
        //If we don't have a double quotation character found yet, increment word count to accomodate
        //strings that are present outside of quotes
        if(quoteCount == 0)
            wordCount++;
        //If we have quoteCount = 1, we will not increment wordCount because we are considering it
        //to be one entire string. Once the second quote is found, we will increment wordCount again.
        else if(quoteCount == 2)
            wordCount++;
    }
    
    //Allocate proper memory size of new command string 
    newQuoteParseCommand = malloc(sizeof(char**) * wordCount);
    for(i = 0; i < wordCount; i++)
        newQuoteParseCommand[i] = malloc(sizeof(char*) * 1024);

    quoteCount = 0;

    //Concatenate without quotes
    for(i = 0; line_words[i] != NULL; i++)
    {
       
        for(j = 0; line_words[i][j] != '\0'; j++)
        {
            
            //If string is one of the outer double quotes
            if((line_words[i][j] == '"') && (line_words[i][j - 1] != '\\'))
            {
                
                quoteCount++;
                while(line_words[i][j] != '\0')
                {   
                    line_words[i][j] = line_words[i][j + 1];
                    j++;
                }
                //Will check if string had two double quotes attatched to it and fix accordingly
                if(line_words[i][strlen(line_words[i]) - 1] == '"' && line_words[i][strlen(line_words[i]) - 2] != '\\')
                {
                    quoteCount++;
                    line_words[i][strlen(line_words[i]) - 1] = line_words[i][strlen(line_words[i])];
                    
                }
                break;
                
            }
            //Need to have logic for removing escape character from string
            if(line_words[i][j] == '\\')
            {
                while(line_words[i][j] != '\0')
                {   
                    line_words[i][j] = line_words[i][j + 1];
                    j++;
                }
                if(line_words[i][strlen(line_words[i]) - 2] == '\\')
                {
                    line_words[i][strlen(line_words[i]) - 2] = line_words[i][strlen(line_words[i]) - 1];
                    line_words[i][strlen(line_words[i]) - 1] = line_words[i][strlen(line_words[i])];
                }
                break;
            }
        }
        

        //If we haven't found quote, then copy linewords string to new adjusted command string to be returned
        if(quoteCount == 0)
        {
            strcpy(newQuoteParseCommand[i], line_words[i]);
            customIterator = i;            
        }
            
        //If we are inside the body of double quotes, then keep concatenate every string in line_words to tempString
        //as if it is one big string instead of separate strings
        else if(quoteCount == 1)
        {
            strcat(tempString, line_words[i]);
            strcat(tempString, " ");
           
        }
        //Once second quote is found, concatenate last string to tempString and copy entire temp string
        //(which is the concatenate string of all strings encased in double quotes) to our new command 
        //to be executed by the caller in main
        else
        {
            
            strcat(tempString, line_words[i]);
            customIterator++;
            strcpy(newQuoteParseCommand[customIterator], tempString);
        }
    
    }
    
    newQuoteParseCommand[customIterator + 1] = 0x0;
    return newQuoteParseCommand;
}


/**
 * @brief 
 * This function written by Dr. Gondree is to exit the program with a syscall
 * and the error message passed.
 * @param s 
 * Error string to be outputed if called
 */

void syserror(const char *s)
{
    extern int errno;
    fprintf(stderr, "%s\n", s);
    fprintf(stderr, " (%s)\n", strerror(errno));
    exit(1);
}