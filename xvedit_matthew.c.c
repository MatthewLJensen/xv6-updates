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

typedef struct Node
{
    char* line;
    struct Node *next;
    //struct Node *prev;
} Node;

typedef struct List
{
    int count;
    struct Node *head;
    struct Node *end;
} List;

char whitespace[] = " \t\r\n\v";
//#define MAX_BUF_SIZE 100
#define MAX_ARGS 10
#define ARG_LEN 30

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
    command = malloc(sizeof *command);
    command->argv = malloc(MAX_ARGS * sizeof command->argv);
    command->argv[0] = malloc(ARG_LEN * sizeof *command->argv);

    int i;
    int arg = 0;
    int offset = 0;
    for (i = 0; buf[i]; i++)
    {
        if (isSpace(buf[i]))
        {
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

    return command;
}

int confirm()
{
    static char conf_buf[100];
    struct command *confirm_response;

    printf(2, "Save Changes (Y/N)?\n");

    while (getcmd(conf_buf, sizeof(conf_buf), 0) >= 0)
    {
        confirm_response = parsecmd(conf_buf);

        if (confirm_response->argc == 1)
        {

            if (strcmp(confirm_response->argv[0], "y") == 0 || strcmp(confirm_response->argv[0], "Y") == 0)
            {
                return 1;
            }
            else if (strcmp(confirm_response->argv[0], "n") == 0 || strcmp(confirm_response->argv[0], "N") == 0)
            {
                return 0;
            }
            printf(2, "Save Changes (Y/N)?\n");
        }
    }

    return 0;
}

//Linked List Functions
char *strdup(const char *old)
{
    char *new = malloc(sizeof(old));
    strcpy(new, old);
    return new;
}

Node* prepend(Node *head, char *new_text)
{
    
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->line = strdup(new_text);
    new_node->next = head;
    
    return new_node;
}

Node* append(Node *head, char *new_text)
{
    //printf(1,"input: %s\n", new_text);
    if ((char *)head == NULL)
    {
        //printf(1,"happens once");
        return prepend(head, new_text);
        
    }
    else
    {
        head->next = append(head->next, new_text);
        return head;
    }
}

Node* insert(Node *head, char *new_text, int pos)
{
    if (pos == 0)
        return prepend(head, new_text);
    else
    {
        head->next = insert(head->next, new_text, pos - 1);
        return head;
    }
}

void print_back(Node *head)
{
    if ((char *)head == NULL)
    {
        printf(1,"***END***\n");
    }
    else
    {
        print_back(head->next);
        printf(1, "%s\n", head->line);
    }
}

void print_forward(Node *head)
{
    if ((char *)head == NULL)
    {
        //printf("***END***\n");
    }
    else
    {
        print_back(head->next);
        printf(1, "%s\n", head->line);
    }
}

Node* read_file(int file, Node *node)
{
    static char buf[255];

    //Try to get a char array that is the exact size of the input
    int n; //Current buffer size from read call
    int totalN = 0;
    char *tempBuf = malloc(sizeof(char));
    while ((n = read(file, buf, sizeof buf)) > 0)
    {
        totalN += n;

        //Allocate extra space
        char *temp = tempBuf;
        tempBuf = malloc(totalN * sizeof(char));

        //Only use temp if we're past first iteration
        if (temp[0] != 0)
            memmove(tempBuf, temp, strlen(temp));

        //Move stuff from buffer and append to temp
        memmove(tempBuf + (totalN - n), buf, n);
        free(temp);
    }
    free(buf);

    int offset = 0;
    printf(1, "totalN: %d", totalN);
    for (int i = 0; i < totalN; i++)
    {
        if (tempBuf[i] == '\n')
        {

            char* temp = malloc(i);
            memset(temp, 0, i - offset + 1);             //Clear out junk data
            memmove(temp, tempBuf + offset, i - offset); //Fill in line with snippet from tempBuf

            //printf(1, "about to append: %s", temp);
            node = prepend(node, temp);
            //printf(1, "just appended: %s", node->line);
            offset = i;
        }
    }
    return node;
}

int main(int argc, char *argv[])
{
    static char buf[100];
    struct command *command;

    int file;

    //Open the file to edit
    if ((file = open(argv[1], O_RDWR)) < 0)
    {
        printf(1, "xvEdit failed to open file %s\n", argv[1]);
        exit();
    }

    //load file into memory
    Node *line = malloc(sizeof(line));
    //char *test = "test";

    //printf(1,"about to append");
    //line = prepend(line, test);
    //printf(1,"output: %s",line->line);
    
    line = read_file(file, line);
    print_back(line);

    //printf(2, "\n\n");
    //for (Node *ln = contents->head; ln != 0; ln = ln->next)
    //  printf(2, "%s", ln->line);
    //printf(2, "\n");

    // Read and run input commands.
    while (getcmd(buf, sizeof(buf), 1) >= 0)
    {

        command = parsecmd(buf);

        //quits xvedit
        if (strcmp(command->argv[0], "quit") == 0)
        {
            if (confirm())
            {
                exit();
            }
        }
    }

    //clean up
    close(file);

    exit();
}



            //char *tempBuf = malloc(sizeof(char));

    //Node **prev_line = &list->head;
        
        char *temp = tempBuf;
        tempBuf = malloc(total * sizeof(char));

        //printf(1, " %s", temp);

        //Move stuff from buffer and append to temp

        printf(1, "before %s", tempBuf);
        memmove(tempBuf, temp, (total * sizeof(char) - 1));
        printf(1, "middle %s", tempBuf);
        memmove(tempBuf, buf, 1);
        printf(1, "after %s", tempBuf);

        //printf(1, "test: %s", buf);
        //printf(1, " %s", tempBuf);
        //printf(1, " %s", temp);

        free(temp);
        printf(1, " %s", tempBuf);





        while ((i = read(file, buf, sizeof(buf))) > 0)
    {

        char *temp = tempBuf;
        tempBuf = malloc(total * sizeof(char));

        //printf(1, " %s", temp);

        //Move stuff from buffer and append to temp

        printf(1, "before %s", tempBuf);
        memmove(tempBuf, temp, (total * sizeof(char) - 1));
        printf(1, "middle %s", tempBuf);
        memmove(tempBuf, buf, 1);
        printf(1, "after %s", tempBuf);

        //printf(1, "test: %s", buf);
        //printf(1, " %s", tempBuf);
        //printf(1, " %s", temp);

        free(temp);
        printf(1, " %s", tempBuf);


void print_list(List *file)
{
    printf(2, "\n\n");
    for (Node *ln = file->head; ln != 0; ln = ln->next)
        printf(2, "%s", ln->line);
    printf(2, "\n");
}