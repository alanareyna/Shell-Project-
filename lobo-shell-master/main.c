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

/*Andrew Notes:

NOTE: It seems that the total number of fork() calls depends on our number of
pipes(|) + 1. So we will need an incrementation of a pipe counter in terms of how
many times it shows up in our line_words.

Note: We might need to look into dynamic memory allocation for allocating enough memory for pidt array
to be passed into pipe() function. This will be dependant on our actual pipe (|) counter that we will be updating 
as we parse through our linwe_words.
For 1 command Tests:
    - We keep track of the number of strings that are going to be 
      stored from our split_cd_line() function. If we find no pipe (|)
      characters, i.e our pipe counter is equal to zero, that implies
      that we should only have one command in our array. This should fork() once

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

int main()
{
    // Buffer for reading one line of input
    char line[MAX_LINE_CHARS];
    // holds separated words based on whitespace
    char* line_words[MAX_LINE_WORDS + 1];

    // Loop until user hits Ctrl-D (end of input)
    // or some other input error occurs
    while( fgets(line, MAX_LINE_CHARS, stdin) ) {
        int num_words = split_cmd_line(line, line_words);

        for (int i=0; i < num_words; i++) {
            printf("%s\n", line_words[i]);
        }
    }
    
    return 0;
}


