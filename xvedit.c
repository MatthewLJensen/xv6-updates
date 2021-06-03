/*
  xv6 text editor
  Matthew Jensen
  March 4, 2021
*/

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define NULL ((char *)0)


struct command
{
    char **argv;
    int argc;
};

struct range
{
    int start;
    int end;
    int length;
};

typedef struct Node
{
    char *line;
    struct Node *next;
    struct Node *prev; //I've decided I don't really need this list to go both ways, but I haven't removed the "prev" piece from all places yet. 
} Node;

typedef struct List
{
    int count;
    int changes;
    struct Node *head;
    struct Node *end;
    char *name;
} List;

char whitespace[] = " \t\r\n\v";
//#define MAX_BUF_SIZE 100
#define MAX_ARGS 10
#define ARG_LEN 30
#define NUM_COMMANDS 13 //change this when adding commands

//FUNCTION DEFINITIONS

//REPL
int getcmd(char *buf, int nbuf, int print_prompt); //gets command from user input
int isSpace(const char c);                         //checks if char is a whitespace character
struct command *parsecmd(const char *buf);         //parses commands

//xvEDIT Commands. See commands array for information on each method.
void end(char *text, List *list);
void add(char *text, List *contents, char *line_num);
void drop(List *contents, char *range);
void list(char *text, List *contents);
void edit(char *text, List *contents, char *line_num);
void find(char *text, List *contents);
void help(struct command *command);
void new_open(char *fd, List *contents);
void save_file(char *fd, List *contents);

//xvEDIT helper functions
int confirm();                                                          //requests user confirmation. Returns 1 for a confirmation
void read_file(int file, List *list);                                   //reads file into linked list
void run_command(struct command *command, List *contents, char *fd);    //runs command
char *commands_to_text(struct command *command, int start);             //takes char ** of commands and puts them back together into words
struct range *convert_range(char *range_input);                         //take a char * and returns an int * with the first value being the starting location and the next value being the number of times to iterate
int num_length(int num);                                                //takes an int and returns the number of numbers are required to write the int in base 10
void string_to_upper(char* string);                                     //takes a string and converts it to an uppercase string
char char_to_upper(char character);                                     //takes a char and returns an uppercase char
void command_converter(struct command *command);                        //iterates through a list of commands to enable a user to enter the shortest ammount of characters required for each command



//Global Varibles

// I just need to add exmaples for the commands   
char *parameters[] = {
    "'filename'\n\tMay be a relative or absolute path, but no spaces",
    "'line_num'\n\t1-based line number",
    "'range'\n\tA consecutive sequence of lines specified by a starting and ending number (both included) separated by a colon, like Python's string/list indexing/slicing (but, unlike in Python, it's 1-based, and the end point is included). All 3 pieces are optional, but you must specify at least 1 of them. Examples: 12 (just 1 line), 3:5 (3 lines), 15: (through the end of the file), :7 (the first 7 lines of the file, same as 1:7 ), : (the whole file!)",
    "'text'\n\tLine of text which may include spaces. Continues through EOL.",
    "'command'\n\tThe name of a command. Run HELP with no parameter to get a list of all commands"
};
char *commands[] = {
    "@END\n\tTakes: text\n\tAppend text as new line at end of file",
    "ADD<\n\tTakes: line_num, text\n\tInsert new line containing text before line line_num",
    "DROP\n\tTakes: range\n\tDelete lines in range",
    "EDIT\n\tTakes: line_num, text\n\tReplace contents of line line_num with text",
    "LIST\n\tTakes: range\n\tOutput lines in range , each preceded by its line number (right-aligned), a colon, and a space",
    "QUIT\n\tTakes: no params required\n\tExit the editor, confirming whether to save changes if any were made",
    "COPY\n\tTakes: range, line_num\n\tDuplicate lines in range at line_num , 'pushing' existing lines at and after line_num 'down'\n\tNot yet implemented",
    "FIND\n\tTakes: text\n\tDisplay line numbers of all lines containing text (casesensitive), separated by a comma and a space",
    "FLIP\n\tTakes: range\n\tReverse order of lines in range (or whole file)\n\tNot yet implemented",
    "HELP\n\tTakes: command\n\tDisplay documentation for command, with examples (or a command list, if command is left out)",
    "OPEN\n\tTakes: filename\n\tLoad contents of filename (confirming loss of any modifications)",
    "SAVE\n\tTakes: filename\n\tStore changes back to original file, unless filename specified",
    "SORT\n\tTakes: range\n\tPut lines in range (or whole file, if absent) in lexicographical order\n\tNot yet implemented"
};

char *commands_alone[] = { //I want to reintegrate this into the commands array as an array within an array, but I ran out of time.
    "@END",
    "ADD<",
    "DROP",
    "EDIT",
    "LIST",
    "QUIT",
    "COPY",
    "FIND",
    "FLIP",
    "HELP",
    "OPEN",
    "SAVE",
    "SORT"
};

int main(int argc, char *argv[])
{
    static char buf[100];
    struct command *command;

    int file;

    printf(1, "Welcome to xvEdit!\n");

    //Open the file to edit
    if ((file = open(argv[1], O_RDWR)) < 0)
    {
        printf(1, "xvEdit failed to open file %s\n", argv[1]);
        exit();
    }

    List *contents = malloc(sizeof *contents); //File data
    read_file(file, contents);
    close(file);

    printf(1, "%d lines read from %s\n", contents->count, argv[1]);

    // Read and run input commands.
    while (getcmd(buf, sizeof(buf), 1) >= 0)
    {
        command = parsecmd(buf);
        run_command(command, contents, argv[1]);

        for (int i = 0; i < command->argc; i++)
        {
            free(command->argv[i]);
        }
        //free(command->argc);
        free(command);
    }



    exit();
}

void run_command(struct command *command, List *contents, char *fd)
{

    if (strcmp(command->argv[0], "@END") == 0)
    {

        char *text = commands_to_text(command, 1);
        end(text, contents);
    }
    else if (strcmp(command->argv[0], "ADD<") == 0)
    {
        char *text = commands_to_text(command, 2);
        add(text, contents, command->argv[1]);
    }
    else if (strcmp(command->argv[0], "DROP") == 0)
    {
        drop(contents, command->argv[1]);
    }
    else if (strcmp(command->argv[0], "EDIT") == 0)
    {
        char *text = commands_to_text(command, 2);
        edit(text, contents, command->argv[1]);
    }
    else if (strcmp(command->argv[0], "LIST") == 0)
    {
        list(command->argv[1], contents);
    }
    else if (strcmp(command->argv[0], "COPY") == 0)
    {
        printf(1, "Unfortunately, this command is not yet implemented.\n");
    }
    else if (strcmp(command->argv[0], "FIND") == 0)
    {
        char *text = commands_to_text(command, 1);
        find(text, contents);
    }
    else if (strcmp(command->argv[0], "FLIP") == 0)
    {
        printf(1, "Unfortunately, this command is not yet implemented.\n");
    }
    else if (strcmp(command->argv[0], "HELP") == 0)
    {
        help(command);
    }
    else if (strcmp(command->argv[0], "OPEN") == 0)
    {
        new_open(command->argv[1], contents);
    }
    else if (strcmp(command->argv[0], "SAVE") == 0)
    {
                
        printf(1, "Save changes without quitting ");
        if (confirm())
        {

            if (command->argc == 1)
            {
                save_file(fd, contents);
            }
            else
            {
                save_file(command->argv[1], contents);
            }
        }
        else
            return;
    }
    else if (strcmp(command->argv[0], "SORT") == 0)
    {
        printf(1, "Unfortunately, this command is not yet implemented.\n");
    }
    else if (strcmp(command->argv[0], "QUIT") == 0)
    {
        if (contents->changes == 0)
            exit();

        printf(1, "Save changes then quit ");
        if (confirm())
        {
            save_file(fd, contents);
            exit();
        }
        else
            exit();
    }
    else
    {
        //run help by default if no valid command is received
    }
}

//Linked List Functions
void append(Node *ln, List *contents)
{
    contents->end->next = ln;
    contents->end = ln;
    contents->end->next = 0;
    contents->count++;
}

//xvedit Commands
void end(char *text, List *contents)
{
    Node *new_line = malloc(sizeof(new_line));
    new_line->line = malloc(1 + strlen(text));
    strcpy(new_line->line, text);
    append(new_line, contents);
    contents->changes += 1; //update changes to track num of changes
}

void add(char *text, List *contents, char *line_num)
{
    int num = atoi(line_num);
    Node *new_line = malloc(sizeof(new_line));
    new_line->line = malloc(1 + strlen(text));
    strcpy(new_line->line, text);

    int i = 1;
    contents->changes += 1; //update changes to track num of changes

    for (Node *ln = contents->head; ln != 0; ln = ln->next)
    {
        if (i + 1 == num)
        {
            new_line->next = ln->next;
            ln->next = new_line;
            contents->count++;
            return;
        }
        i++;
    }

}


void drop(List *contents, char *range)
{ //deleting range starting with first line doesn't currently work. Deleting everything doesn't currently work.
    
    struct range *converted_range = convert_range(range);
    //printf(1,"start: %d\n", converted_range->start);
    //printf(1,"end: %d\n", converted_range->end);
    //printf(1,"length: %d\n", converted_range->length);

    //calculate the number of lines to drop
    if      (converted_range->length == 0 && converted_range->start != 0 && converted_range->end == 0)
        converted_range->length = contents->count - converted_range->start; //This value is sometimes incorrect. Do I need to add 1 to count, because count starts at 0?
    else if (converted_range->length == 0 && converted_range->start == 0 && converted_range->end != 0)
        converted_range->length = converted_range->end;
    else if (converted_range->length == 0 && converted_range->start == 0 && converted_range->end == 0)
        converted_range->length = contents->count;

    printf(1, "Drop %d pages ", converted_range->length);
    if (!confirm())
        return;

    contents->changes += 1;
    Node *start = contents->head;
    for (int i = 0; i < contents->count; i++){

        if (converted_range->end == 0 && converted_range->start != 0 && i == converted_range->start - 2)
        {
            //this is a case where there is no end digit supplied, this needs to delete every line starting with the start line.
                
            Node *temp1;
            Node *temp2;

            temp1 = start->next;
            start->next = 0;

            while (temp1->next != 0)
            {
                //printf(1,"deleting line");
                temp2 = temp1;
                temp1 = temp1->next;
                free(temp2);
                contents->count--;
            }
            free(temp1);
            return;
        }
        else if (converted_range->end != 0 && converted_range->start == 0 && i <= converted_range->end - 1)
        {
            //this is a case where there is no beginning digit supplied, this needs to delete every line until the end line.
            contents->head = start->next;
            contents->count--;
            free(start);
        }
        else if (converted_range->end == 0 && converted_range->start == 0)
        {
            //this is a case where there are no start or end digits supplied. This means everything needs to be deleted. This isn't working at the moment
            //contents->head = 0;
            //contents->count = 0;
        }
        else if (i >= converted_range->start - 2 && i <= converted_range->end && converted_range->start != 0 && converted_range->end != 0)
        {
            Node *temp1;
            temp1 = start;
            

            while (i <= converted_range->end - 1)
            {
                Node *temp2;
                temp2 = temp1;
                temp1 = temp1->next;
                free(temp2);
                contents->count--;                
                i++;
            }
            start->next = temp1;
            return;
        }
        start = start->next;
    }
}

void edit(char *text, List *contents, char *line_num)
{
    int num = atoi(line_num);
    Node *new_line = malloc(sizeof(new_line));
    new_line->line = malloc(1 + strlen(text));
    strcpy(new_line->line, text);


    int i = 1;
    for (Node *ln = contents->head; ln != 0; ln = ln->next)
    {
        if (i == num)
        {
            ln->line = new_line->line;
            return;
        }
        i++;
    }

    contents->changes += 1;
}

void new_open(char *fd, List *contents)
{
    int file;
    //Open the file to edit
    if ((file = open(fd, O_RDWR)) < 0)
    {
        printf(1, "xvEdit failed to open file %s\n", fd);
        return;
    }


    printf(1,"Unsaved changes will be lost. Open new file ");
    if (!confirm()){
        return;
    }



    read_file(file, contents);
    close(file);

    printf(1, "%d lines read from %s\n", contents->count, fd);
}

void save_file(char *fd, List *contents)
{
    int file;
    //Open the file to edit
    if ((file = open(fd, O_CREATE|O_RDWR)) < 0)
    {
        printf(1, "xvEdit failed to open file %s\n", fd);
        return;
    }


    for (Node *ln = contents->head; ln != 0; ln = ln->next) //I think this is adding an extra line at the end, at least sometimes
    {
        int len = strlen(ln->line);
        char *line = malloc((len + 1) * sizeof *line);
        strcpy(line, ln->line);
        line[len] = '\n';
        write(file, line, len + 1);
        free(line);
    }
    close(file);


    printf(1, "%d lines saved to %s\n", contents->count, fd);
}



void list(char *range, List *contents)
{ //
    int i = 1;

    struct range *converted_range = convert_range(range);
    int line_num_length = 0;

    if (converted_range->end != 0)
        line_num_length = num_length(converted_range->end);
    else
        line_num_length = num_length(contents->count); //I don't know what the end is until I loop through them, so if end is 0, I know I will be looping through all of the lines, meaning I need to use the size of contents.

    //printf(1, "Start: %d, end: %d, length: %d", converted_range->start, converted_range->end, converted_range->length);

    printf(1, "\n\n");



    for (Node *ln = contents->head; ln != 0; ln = ln->next) //this is way more complicated/repetetive than it needs to be, but it works. Please forgive me Prof O.
    {
        int current_length = num_length(i);
        int white_space_required = line_num_length - current_length;
        

        if(converted_range->end == 0 && converted_range->start != 0 && i >= converted_range->start)
        {
            //this is a case where there is no end digit supplied, this needs to print every line starting with the start line.

            for (int j = 0; j < white_space_required; j++)
                printf(1, " ");
            
            printf(1, "%d: ", i);
            printf(1, "%s", ln->line); 
            printf(1, "\n");
        }

        if(converted_range->end != 0 && converted_range->start == 0 && i <= converted_range->start)
        {
            //this is a case where there is no beginning digit supplied, this needs to print every line until the end line.

            for (int j = 0; j < white_space_required; j++)
                printf(1, " ");
            
            printf(1, "%d: ", i);
            printf(1, "%s", ln->line); 
            printf(1, "\n");
        }

        if(converted_range->end == 0 && converted_range->start == 0)
        {
            //this is a case where there are no start or end digits supplied. This means everything needs to print.

            for (int j = 0; j < white_space_required; j++)
                printf(1, " ");
            
            printf(1, "%d: ", i);
            printf(1, "%s", ln->line); 
            printf(1, "\n");
        }

        if(i >= converted_range->start && i <= converted_range->end)
        {
            for (int j = 0; j < white_space_required; j++)
                printf(1, " ");
            
            printf(1, "%d: ", i);
            printf(1, "%s", ln->line); 
            printf(1, "\n");
        }
      
        i++;
    }
    printf(1, "\n");
}


void find(char *text, List *contents)
{
    int line_matches[contents->count];//declares an array large enough to hold every line number. There's probably a better way to do this dynamically.
    int i = 1; //keeps track of the line the file is looking at
    int m = -1; //position of the line number to be placed in the line_matches array.

    for (Node *ln = contents->head; ln != 0; ln = ln->next) //loops through each line in the file
    {
        int j = 0; //position in the text being searched
        int k = 0; //position in the text being searched for
        int found = 0;

        while (ln->line[j] != '\0') //loops through each char in the line
        {
            if (ln->line[j] == text[k])
            {
                k++;
            }
            else
            {
                k = 0;
            }

            if (k == strlen(text))
                found = 1;

            j++;
        }

        if (found == 1) //I use this "boolean" so that I can add the found line number outside of the loop. This allows me to keep from breaking the loop through each line when a match is found.
        {
            m++; //It's kinda weird putting this first, but I use this later to determine how many succesful line matches there were, and if it increments after the fact, it's off by one.
            line_matches[m] = i;
            found = 0;
        }

        i++;
    }


    if (m > 0)
    {
        printf(1, "\"%s\" is found on line(s): ", text);
        for (int i = 0; i <= m; i++)
        {
            printf(1,"%d", line_matches[i]);
            if (i != m)
            {
                printf(1, ", ");
            }
            
        }
    }
    else{
        printf(1, "Sorry, \"%s\" was unable to be located in the file", text);
    }

    printf(1, "\n");
}

void help(struct command *command)
{


    printf(1, "\nWelcome to XVEDIT. This is the help tool.\n\n");
    if (command->argc < 2)
    {
        //They want help on all commands

        printf(1, "PARAMETERS:\n\n");
        for (int i = 0; i < sizeof(parameters)/sizeof(char*); i++) //I got this nifty method for obtaining the size of a char[] from Miro.
        {
            printf(1, "    %s\n\n", parameters[i]);
        }
        
        printf(1, "\nFUNCTIONS:\n\n");
        for (int i = 0; i < sizeof(commands)/sizeof(char*); i++)
        {
            printf(1, "    %s\n\n", commands[i]);
        }
    }
    else if (command->argc == 2)
    {

        if (strcmp(command->argv[0], "@END") == 0) printf(1, "%s", commands[0]);
        else if (strcmp(command->argv[1], "ADD<") == 0) printf(1, "%s", commands[1]);
        else if (strcmp(command->argv[1], "DROP") == 0) printf(1, "%s", commands[2]);
        else if (strcmp(command->argv[1], "EDIT") == 0) printf(1, "%s", commands[3]);
        else if (strcmp(command->argv[1], "LIST") == 0) printf(1, "%s", commands[4]);
        else if (strcmp(command->argv[1], "QUIT") == 0) printf(1, "%s", commands[5]);
        else if (strcmp(command->argv[1], "COPY") == 0) printf(1, "%s", commands[6]);
        else if (strcmp(command->argv[1], "FIND") == 0) printf(1, "%s", commands[7]);
        else if (strcmp(command->argv[1], "FLIP") == 0) printf(1, "%s", commands[8]);
        else if (strcmp(command->argv[1], "HELP") == 0) printf(1, "%s", commands[9]);
        else if (strcmp(command->argv[1], "OPEN") == 0) printf(1, "%s", commands[10]);
        else if (strcmp(command->argv[1], "SAVE") == 0) printf(1, "%s", commands[11]);
        else if (strcmp(command->argv[1], "SORT") == 0) printf(1, "%s", commands[12]);
        else printf(1, "\nNo command found by that name. Run help with no parameters to see list of commands");
        printf(1,"\n\n");
    }
    else
    {
        printf(1, "\nHELP takes only 1 parameter\n\n");
    }
}

//Helper Functions
int num_length(int n)
{ //https://stackoverflow.com/a/1068937/5369588 (raw speed version)
    if (n < 10) return 1;
    if (n < 100) return 2;
    if (n < 1000) return 3;
    if (n < 10000) return 4;
    if (n < 100000) return 5;
    if (n < 1000000) return 6;
    if (n < 10000000) return 7;
    if (n < 100000000) return 8;
    if (n < 1000000000) return 9;
    return 0;
}


void read_file(int file, List *contents)
{
    contents->count = 0; //resets the counter
    contents->changes = 0; //resets the counter
    static char buf[512];
    int n;
    int total = 0;
    char *tempBuf = malloc(sizeof(char));
    while ((n = read(file, buf, sizeof buf)) > 0)
    {
        
        total += n;

        char *temp = tempBuf;
        tempBuf = malloc(total * sizeof(char));
        
        if (temp[0] != 0)
        {
            
            memmove(tempBuf, temp, strlen(temp));
            
        }
        //Move contents into tempBuf
        
        memmove(tempBuf + (total - n), buf, n);
        
        free(temp);

    }
    //free(buf); I'm not sure that I actually need to free this.    
    

    int offset = 0;
    Node **prev_line = &contents->head;
    //printf(1, "total: %d", total);
    for (int i = 0; i <= total; i++)
    {
        if (tempBuf[i] == '\n' || i == total)
        {
            

            Node *ln = malloc(sizeof *ln);
            ln->line = malloc((i - offset + 1) * sizeof(char));
            memset(ln->line, 0, i - offset + 1);//clears old data
            memmove(ln->line, tempBuf + offset, i - offset);//puts the line into Node

        
            if (ln->line[0] == '\n')//lazily store each empty line without '\n' with some simple pointer addition
                ln->line += 1;

            //put node into linked list
            ln->prev = *prev_line;
            contents->end = ln;
            *prev_line = ln;
            prev_line = &ln->next;

            contents->count++;

            offset = i;
        }
    }
    free(tempBuf);
}

char *commands_to_text(struct command *command, int start)
{
    char *text = malloc(command->argc * 10 * sizeof(char)); //this isn't the best right way to do this. Oh well. I don't know what I'm doing...
    
    int offset = 0;
    for (int i = start; i < command->argc; i++)//loops through each word in the command
    {
        
        int j;
        for (j = 0; j < strlen(command->argv[i]); j++) //loops through each char in each argument
        {
            text[offset + j] = command->argv[i][j];
            //printf(1, "%d", (offset + j));
        }
        
        if (i < (command->argc - 1))
        {
            //adds spacing between words
            offset += j;
            text[offset] = ' ';
            offset += 1;
        }
        else if (i == (command->argc - 1))
        {
            //add string terminator
            offset += j;
            text[offset] = '\0';
            offset += 1;
        }
    }
    
    return text;
}

struct range *
convert_range(char *range_input)
{
    struct range *range_output = malloc(sizeof(range_output));

    char *start = malloc(10);
    char *end = malloc(10);

    int start_int = (sizeof(int));
    int end_int = (sizeof(int));

    for (int i = 0; i < strlen(range_input); i++)
    {
        if (range_input[i] == ':')
        {
            if (i == 0 && strlen(range_input) == 1)
            {
                //only : in command. Range is entire list
                start_int = 0;
                end_int = 0;
            }
            else if (i == 0 && strlen(range_input) > 1)
            {
                //No first range number, starting from beginning.

                memmove(end, range_input + i + 1, sizeof(int)); //strlen(range_input - i));

                start_int = 0;
                end_int = atoi(end);
            }
            else if (i != 0 && strlen(range_input) == i)
            {
                //No last range number, ends at end of list

                memmove(start, range_input, sizeof(int));

                start_int = atoi(start);
                end_int = 0;
            }
            else
            {
                //All three present
                memmove(start, range_input, sizeof(int));
                memmove(end, range_input + i + 1, sizeof(int)); //strlen(range_input - i));

                start_int = atoi(start);
                end_int = atoi(end);
            }

            range_output->start = start_int;
            range_output->end = end_int;

            if (end_int != 0 && start_int != 0)
            {
                range_output->length = end_int - start_int + 1;
            }
            else
            {
                range_output->length = 0;
            }

            return range_output;
        }
    }

    //There is no :, so the range is only 1 line
    memmove(start, range_input, sizeof(int));
    range_output->start = atoi(start);
    range_output->end = atoi(start);
    range_output->length = 1;


    //printf(1,"start: %d\n", range_output->start);
    //printf(1,"length: %d\n", range_output->length);
    return range_output;
}

void string_to_upper(char *string) //see: https://forgetcode.com/c/567-change-string-to-upper-case-without-strupr
{
   while(*string)
   {
       if ( *string >= 'a' && *string <= 'z' )
       {
          *string = *string - 32;
       }
       string++;
   }
}

char char_to_upper(char character) //see: https://forgetcode.com/c/567-change-string-to-upper-case-without-strupr
{
    if (character >= 'a' && character <= 'z' )
    {
        character = character - 32;
    }
    return character;
}

void command_converter(struct command *command)
{
    //printf(1,"\n Test for erasure: %s", commands[0]);
    //printf(1,"\n The given command is: %s", command->argv[0]);    
    int count = 0;
    char *desired_command = "";
    char temp_list[NUM_COMMANDS][5];

    //copy commands array
    for (int i = 0; i < NUM_COMMANDS; i++)
    {
        //temp_list[i] = malloc(5 * sizeof(char));
        strcpy(temp_list[i], commands_alone[i]);
    }

    //delete rows that don't match
    for (int i = 0; i < strlen(command->argv[0]); i++)
    {
        for (int j = 0; j < NUM_COMMANDS; j++)
        {
            if (commands_alone[j][i] != command->argv[0][i])
            {
                temp_list[j][0] = '\0';
            }
        }
    }

    //counts the items not starting with '\0'
    for (int i = 0; i < NUM_COMMANDS; i++)
    {
        if (temp_list[i][0] != '\0')
        {
            count++;
            desired_command = temp_list[i];
        }
        //printf(1,"\nitem %d: %s\n", i, temp_list[i]);
    }

    if (count > 1)
    {
        printf(1, "Unable to determine the desired command. Try being more specific.\n");
    }
    else if (count == 0)
    {
        printf(1, "The specified command does not appear to exist.\n");
    }
    else
    {
        strcpy(command->argv[0], desired_command);
        //printf(1, "Desired command: %s\n", desired_command);
    }
}

int confirm()
{ //Slight problem, since I didn't want to put an int into a string, I am not passing the text that comes before the '(Y/N)?'. Because of this, if invalid text is entered, the next line will not have this preceding text.
    static char conf_buf[100];
    

    printf(2,"(Y/N)? ");

    while (getcmd(conf_buf, sizeof(conf_buf), 0) >= 0)
    {
        char response = conf_buf[0];
        
        response = char_to_upper(response);
        if (conf_buf[1] == '\n')
        {

            if (response == 'Y')
            {
                return 1;
            }
            else if (response == 'N')
            {
                return 0;
            }
        }
        printf(2, "(Y/N)? ");
    }

    return 0;
}

//REPL Functions
int getcmd(char *buf, int nbuf, int print_prompt)
{
    if (print_prompt)
    {
        printf(2, "xvEdit> ");
    }

    memset(buf, 0, nbuf);
    gets(buf, nbuf);
    if (buf[0] == 0) // EOF
        return -1;
    return 0;
}

int isSpace(const char c)
{
    if (strchr(whitespace, c) == 0)
        return 0;
    else
        return 1;
}

struct command *
parsecmd(const char *buf)
{
    struct command *command;
    command = malloc(sizeof(struct command));
    command->argv = malloc(MAX_ARGS * sizeof(char*));
    command->argv[0] = malloc(5 * sizeof(char));

    int i;
    int arg = 0;
    int offset = 0;
    for (i = 0; buf[i]; i++)
    {
        if (isSpace(buf[i]))
        {
            command->argv[arg][i - offset] = '\0';
            arg++;
            command->argv[arg] = malloc(30 * sizeof(char));
            offset = i + 1;
            continue;
        }
        command->argv[arg][i - offset] = buf[i];
    }
    command->argc = arg;



    

    //sets excess args to null
    for (int i = arg; i < MAX_ARGS; i++)
    {
        command->argv[i] = '\0';
    }

    //printf(1, "get in 4");
    //adjust command name in case they used a shortcut
    string_to_upper(command->argv[0]);
    if (strlen(command->argv[0]) < 4 && strlen(command->argv[0]) > 0)
        command_converter(command);


    return command;
}

