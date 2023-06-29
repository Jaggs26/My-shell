//BT20CSE071
/********************************************************************************************
This is a template for assignment on writing a custom Shell. 

Students may change the return types and arguments of the functions given in this template,
but do not change the names of these functions.

Though use of any extra functions is not recommended, students may use new functions if they need to, 
but that should not make code unnecessorily complex to read.

Students should keep names of declared variable (and any new functions) self explanatory,
and add proper comments for every logical step.

Students need to be careful while forking a new process (no unnecessory process creations) 
or while inserting the single handler code (should be added at the correct places).

Finally, keep your filename as myshell.c, do not change this name (not even myshell.cpp, 
as you not need to use any features for this assignment that are supported by C++ but not by C).
*********************************************************************************************/
#include <stdio.h>
#include <string.h>         
#include <stdlib.h>			//exit() 
#include <unistd.h>			//fork(), getpid(), exec() 
#include <sys/wait.h>		//  for wait()
#include <signal.h>			//  for signal()
#include <fcntl.h>			//  for close(), open()
char** parseInput(char* line) //to remove spaces and arrange them in array like will parse the input string into multiple commands 
	{
		char** tokens=malloc(sizeof(char*));
		char* token=malloc(sizeof(char));
		int position=0;// token_count
		while((token=strsep(&line," "))!=NULL)
			{
				tokens[position]=token;// return the char in the line to the array
				position++;// increments and passes to next char 
			}
		tokens[position]=NULL;// setting the n th postion to null 
		free(token);
		return tokens;	
	}

void executecommand(char** arugument)
	{     //// This function is used to  fork a new process to execute a command
		if(strcmp(arugument[0],"cd")==0) //for cd command
			{ 
			    
				chdir(arugument[1]); //chdir - used fo changing directory
				
				
			}
		else
			{
				int pid=fork();
				if(pid==0)
					{
						//child process
						if(execvp(arugument[0],arugument)<0) //if  execution is failed  it returns negative
							{
                                printf("Shell: Incorrect command\n");
								exit(1);
							}
					}
                else if(pid<0)
                    {
                        //forking child process failed.
                        exit(1);
                    }
				else
					{
					
						wait(NULL); //  parent process wait till child process terminates.
					}
                return;
			}
	}
void executeParallelCommands(char** arugument)
{ // This function will run multiple commands in parallel
    int index=0;
    int previndex = index;
    //while loop because of multiple && commands
    while(arugument[index] != NULL)
    {
        while(arugument[index]!=NULL && strcmp(arugument[index],"&&") != 0)
        {
            index++;
        }
        arugument[index] = NULL;
        if(strcmp(arugument[previndex],"cd")==0) //for cd command
            {
                chdir(arugument[previndex+1]); //chdir - changing directory
            }
        else
            {
                //creating a child by fork
                int pid= fork();
                if(pid < 0)
                {
                    exit(1); // error creating child process
                }
                else if(pid == 0) // inside child process
                {
                    if(execvp(arugument[previndex],&arugument[previndex]) < 0)
                    {
                        printf("Shell: Incorrect command\n");
                        exit(1); // error executing the process
                    }
                }
            }
        index++;
        previndex = index;
    }
    while(wait(NULL)>0);
}
void executeSequentialCommands(char** arugument)
{
    int index=0;
    int previndex = index;
    //while loop because of multiple ## commands
    while(arugument[index] != NULL)
    {
        while(arugument[index]!=NULL && strcmp(arugument[index],"##") != 0)
        {
            index++;
        }
        arugument[index] = NULL;
        executecommand(&arugument[previndex]);
        index++;
        previndex = index;

    }
}
void executeCommandRedirection(char** arugument)
{
    int index=0;
    while(arugument[index]!=NULL && strcmp(arugument[index],">")!= 0)
    {
        index++;
    }
    arugument[index]=NULL;
    
    int pid=fork();
    if(pid<0)
    {
        exit(1);
    }
    else if(pid==0)
    {
        close(STDOUT_FILENO);  // closing current output
        open(arugument[index+1],O_CREAT|O_RDWR|O_APPEND); // below executing commands are storing in the file(arugument[last])
        if(execvp(arugument[0],arugument)<0)
        {
            printf("Shell: Incorrect command\n");
            exit(1); // error executing the process
        }
    }
    else
    {
        wait(NULL);
    }
}
int main()
	{
        signal(SIGINT, SIG_IGN); //Ignore SIGINT signal(for ^C)
        signal(SIGTSTP, SIG_IGN); //Ignore signal (for ^Z)
       
		long unsigned int size=0; //or we can use size_t is of type long unsigned
		char* input=NULL;
		while(1) // This loop will keeep shell running until user exits
			{
			 //to print in format- currentworkingdirectory$.
	           char pwd[1018]; //pwd is the variable to store the present working directory;
		       getcwd(pwd,sizeof(pwd)); //getcwd - gets the current working directory;
	         	printf("%s$",pwd); //print the present working directory.
				getline(&input,&size,stdin); //stdin - standard input from keyboard
                for(int i=0;input[i] != '\0';i++)
                {
                    if(input[i] == EOF || input[i] == '\n') // if the index becoms end of file or a new line is enterd then we have to enter null in the place of that index 
                    {
                        input[i] = '\0';
                    }
                }
                char** arugument=NULL;
        		arugument	=parseInput(input);
        		if(strcmp(arugument[0],"exit")==0) //when user uses exit state
        			{
                        printf("Exiting shell...\n");
                        exit(0);
        			}
                int i=0,state=0;
                while(arugument[i] != NULL && state==0)
                {
                    //checking for && , ## and > and executing  their respected functions
                    if(strcmp(arugument[i],"&&") == 0)
                    {
                        state = 1; // for parallel execution of commands
                    }
                    else if(strcmp(arugument[i],"##") == 0)
                    {
                        state = 2; // for sequentially execution of commands 
                    }
                    else if(strcmp(arugument[i],">") == 0)
                    {
                        state = 3; // for output redirection 
                    }
                    i++;
                }
                 if(state == 1)
                 {
                    executeParallelCommands(arugument);        // when user wants to run multiple commands in parallel i.e commands seprated by &&
                 }
                else if(state == 2)
                {
                    executeSequentialCommands(arugument);    //  when user wants to run multiple commands sequentially i.e commands separated by ##
                }
                else if(state == 3)
                {
                    executeCommandRedirection(arugument);    // This function is used when user wants to redirect the output of one command to another file.
                }
                else if(strcmp(input,"")==0)
                {
                    // if user dint enter any command it just does nothing and goes to next line.
                }
                else
                {
                    executecommand(arugument);
                }
                
			}       
    }
