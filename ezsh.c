//Matthew Jensen
//Last Edit, February 21, 2021

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

struct command { //Miro suggested I use a struct in this manner, especially because it makes it easier to implement history
  char** argv;
  int argc;
};

char whitespace[] = " \t\r\n\v";
//#define MAX_BUF_SIZE 100
#define MAX_ARGS 10
#define ARG_LEN 30

int
getcmd(char *buf, int nbuf, int command_num)
{
  printf(2, "%d | EZ$ ", command_num); //Prints the prompt
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

int isSpace(const char c) {
    if ( strchr(whitespace, c) == 0 ) //checks for whitespace
        return 0;
    else
        return 1;
}

struct command*
parsecmd(const char *buf){
  struct command* command;
  command = malloc(sizeof *command); //allocations space for the struct
  command->argv = malloc(MAX_ARGS * sizeof command->argv); //allocates space for a command with 10 arguments
  command->argv[0] = malloc(ARG_LEN * sizeof *command->argv); //allocates space for the first char* in argv
  
  int i;
  int arg = 0;
  int offset = 0;
  for (i=0; buf[i]; i++){//iterate through the array of chars
    if (isSpace(buf[i])){//If there is a whitespace, then...
      arg++;
      command->argv[arg] = malloc(30 * sizeof (char));
      offset = i + 1; //This offset effectively stores the starting place for the current string
      continue;
    }
    command->argv[arg][i - offset] = buf[i]; //this slices from the subset to the current iterative location

  }
  command->argc = arg; //stores the number of arguments


  //sets excess args to null, starting at last argument
  for (int i = arg; i < MAX_ARGS; i++){
    command->argv[i] = '\0';
  }

  return command;
}


int
main(void)
{
  static char buf[100];
  int command_num = 0;
  int history_size = 10;
  struct command* command;
  struct command** history = malloc(history_size * sizeof(command));

  // Read and run input commands.
  while(getcmd(buf, sizeof(buf), command_num) >= 0){


    command = parsecmd(buf);
    int background = 0;

    //adds command to history
    history[command_num] = command;


    //This handles changing directories
    //This needs to be a built-in command, because the current working directory is integral in how the shell executes commands. This is why it can't be called from a child process.
    if (strcmp(command->argv[0],"cd") == 0){
      printf(2,"changing dirs\n");
      if (command->argc < 2){
        printf(2, "cd takes more than 1 argument.");
      }else if(chdir(command->argv[1]) != 0){
        printf(2, "An error occurred while changing directory");
      }
      continue;
    }

    //this handles history requests
    if (command->argv[0][0] == '#'){
      //printf(2,"running historical command\n");

      //this changes the string to an int to be able to call the command in the history array.      
      int requested_command_num = atoi(command->argv[0] + 1);
      //printf(2, "requested command: %s\n", history[requested_command_num]->argv[0]);
      command = history[requested_command_num];

      //this overwrites the history so that is doesn't list the #, but rather the command that was called
      history[command_num] = command;
    }

    //this handles history requests
    if (command->argv[0][strlen(command->argv[0]) - 1] == '&'){
      //printf(2,"running command in background\n");
      
      //remove the ampersand from the argument before running it
      command->argv[0][strlen(command->argv[0]) - 1] = '\0';
      
      background = 1;
    }

    //exits from mjsh
    if (strcmp(command->argv[0],"exit") == 0){
      printf(2,"exiting mjsh\n");
      exit();
    }


    //this executes the command
    if (fork() == 0){ 
      exec(command->argv[0], command->argv);

      //if the exec fails, this will exit the child process
      exit();
    }
    //if the command isn't supposed to run in the background, wait.
  
    if (background == 0){
      wait();
    }else{
      //printf(2, "not waiting...");
    }
    
    
    //increments the command number
    command_num++;
    if (command_num == (history_size )){
      //history size limit reached. Starting over from 1 and rewriting the history back up through the limit
      command_num = 0;
      printf(2,"history restarting... counter has been reset to 0\n");
    }
    
  }

  exit();
}
