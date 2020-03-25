#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

// Assume that each command line has at most 256 characters (including NULL)
#define MAX_CMDLINE_LEN 256

// Assume that we have at most 16 pipe segments
#define MAX_PIPE_SEGMENTS 16

// Assume that each segment has at most 256 characters (including NULL)
#define MAX_SEGMENT_LENGTH 256
 
// This function will be invoked by main()
// TODO: Implement the multi-level pipes below
void process_cmd(char *cmdline);

// tokenize function is given
// This function helps you parse the command line
//
// Suppose the following variables are defined:
//
// char *pipe_segments[MAX_PIPE_SEGMENTS]; // character array buffer to store the pipe segements
// int num_pipe_segments; // an output integer to store the number of pipe segment parsed by this function
// char cmdline[MAX_CMDLINE_LEN]; // The input command line
//
// Sample usage of this tokenize function:
//
//  tokenize(pipe_segments, cmdline, &num_pipe_segments, "|");
// 
void tokenize(char **argv, char *line, int *numTokens, char *token);


/* The main function implementation */
int main()
{
    char cmdline[MAX_CMDLINE_LEN];
    fgets(cmdline, MAX_CMDLINE_LEN, stdin);
    process_cmd(cmdline);
    return 0;
}

/* 
    Implementation of process_cmd

    TODO: Clearly explain how you implement process_cmd in point form. For example:

    Step 1: I count the number of commands that is going to be executed by this program by using the fucntion tokenize. We input the cmdline (command line) and delimiting 
            the char "|", then store it as segments.
    Step 2: I create loops corresponding to the number of commands that I got in the Step 1.
        //within the loops
        Step 2.1: If it is not the last command, I will create a pipe to connect it with previous command 
        Step 2.2: Then, I will fork() the process since we are going to execute more than 1 commands. We need a new process as the previous process will be used
                  to store the previous execvp and then will be passed to the new process through pipe.
            //Piping process
            Step 2.2.1: If it is not the last command, we will redirect the stdout of the pipe to the input of the pipe which is pdfs[1]. We do it because we
                        want to use the pipe to move the current data to the next pipe.
            Step 2.2.2: If it is not the first command, we will close the input and use the data contained in the pipe as the input of the current process. We
                        direct the output of the pipe to the imput of the current process.
            //segments extracting and executing
            Step 2.2.3: After we configure how the pipe will work, we will extract the segments which could include space between them like "ls -l -h". To do
                        this, we basically do the same step as step 1 but we use " " (space) as the delimiter and segments as the input. We store it in char* 
                        containers called strings.
            Step 2.2.4: Now we have array named strings which contain all the executable command. We use execvp and pass the array so that it will execute 
                        after all the string have been extracted. The array will continue to grow until the loop is finished and then it will execute the 
                        command.
        Step 2.3: Since we want ot use the previous pipe output as the new input for each loop, in each loop we will store the output of the pipe (pdfs(0)) and
                  store it in a variable called 'in'. This variable is the one used as the input in Step 2.2.2. We close pdfs[1] since we are not using it. 
    Step 3: We use wait(0) so that it will wait the program to finish executing which in my case are contained on the child's process.

 */
void process_cmd(char *cmdline)
{   char* segments[MAX_PIPE_SEGMENTS];
    int num=0;
    int* numTokens = &num;
    //Step 1
    tokenize(segments ,cmdline, numTokens,"|");
    int i, in, out = dup(1);    // save the original standard output
    //Step 2
    for (i = 0; i < num; i++)
    {
        int pdfs[2];
        // Step 2.1
        if (i < num-1){ 
            pipe(pdfs);
            } 
        // Step 2.2 
        pid_t pid = fork();
        if (pid == 0)
        {
            // Step 2.2.1
            if(i < num-1){
                dup2(pdfs[1], 1);
                close(pdfs[1]);
            }
            // Step 2.2.2
            if(i != 0){
                dup2(in, 0);
                close(in);
            }
            // Step 2.2.3
            char* strings[MAX_SEGMENT_LENGTH];
            int y = 0;
            int* numStrings = &y;
            tokenize(strings, segments[i], numStrings, " ");
            execvp(strings[0], strings);
        }
        // Step 2.3
        close(pdfs[1]);
        in = pdfs[0]; // previous pipe output as the new in
        
    }
    //Step 3
    wait(0);
    // Delete this line to start your work
    printf("Debug: %s\n", cmdline);
}

// Implementation of tokenize function
void tokenize(char **argv, char *line, int *numTokens, char *delimiter)
{
    int argc = 0;
    char *token = strtok(line, delimiter);
    while (token != NULL)
    {
        argv[argc++] = token;
        token = strtok(NULL, delimiter);
    }
    argv[argc++] = NULL;
    *numTokens = argc - 1;
}