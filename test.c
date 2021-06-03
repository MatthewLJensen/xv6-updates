#include "types.h"
#include "user.h"
#include "date.h"
#include "fcntl.h"

#define WAIT 300
#define NULL ((void*) 0)

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

typedef struct cronarray
{
    int size;
    int run_when_matched[10];
} cronarray;

typedef struct cronjob
{
    int *minute;
    int *hour;
    int *day;
    int *month;
    int *day_of_week;
} cronjob;

struct command {
  char** argv;
  int argc;
};

//funciton defs
void read_file(int file, List *list);
void get_cron_details(Node *ln, cronarray *cron, struct command *command);
int int_to_time(struct rtcdate r, int i);
int day_to_int(char *day);
int month_to_int(char *month);
int is_digit(char testchar); //returns 1 if a char is a digit

//global vars
char *months[] = {
    "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"
};
char *days[] = {
    "sun", "mon", "tue", "wed", "thu", "fri", "sat"
};
char *labels[] = {
    "minute", "hour", "day", "month", "day_of_week"
};
char *NUMBERS = "123456789";

int main(int argc, char *argv[])
{
    struct rtcdate r;
    struct cronarray cron[5];
    struct command *command = malloc(sizeof(struct command));
    struct List *contents = malloc(sizeof(struct List));
    //int cronjob;
    int file;
    int iterations = 0;
 
    //synchronize to run at the beginning of every minute
    /*date(&r);
    if (r.second > 0)
    {
        sleep(100 * (60 - r.second));
    }*/

    //testing is_digit
    //printf(1, "checking 1: %d. checking k: %d", is_digit('1'), is_digit('k'));
    //printf(1, "checking mon: %d. checking sat: %d", day_to_int("mon"), day_to_int("sat"));
    //printf(1, "checking jan: %d. checking may: %d", month_to_int("jan"), month_to_int("may"));





    for(;;){
        
        date(&r); //grab the current time info
        printf(1, "\n\nThe current time: %d:%d:%d. Day: %d. Month: %d\n\n", r.hour, r.minute, r.second, r.day, r.month);
        printf(1, "this is iteration # %d\n", iterations);


        //Open cron file and read contents
        if ((file = open("cron", O_RDWR)) < 0)
        {
            printf(1, "could not open cron file\n");
            exit();
        }

        
        printf(1, "getting here 1");                    
        read_file(file, contents);
        close(file);



        //check for matching time entries
        Node *ln = contents->head;
        for (int i = 0; i < contents->count; i++)
        {
            
            if (i != 0){
                ln = ln->next;
            }

            //filter out commented lines
            if (ln->line[0] == '#' || strlen(ln->line) < 1)
            {
                continue;
            }

            get_cron_details(ln, cron, command);

            
            //print results for that line in cron. Uncomment for testing.
            /*
            printf(1, "printing results\n");
            for (int i = 0; i < 5; i++)
            {
                for (int j = 0; j < cron[i].size; j++)
                {
                    printf(1, "%s:%d\n",labels[i], cron[i].run_when_matched[j]);
                }
            }*/
            

            //check if cron matches
            int execute = 1;
            
            for (int i = 0; i < 5; i++)
            {
                int matches = 0;
                for (int j = 0; j < cron[i].size; j++)
                {

                    if (cron[i].run_when_matched[0] != -1)
                    {
                        if(int_to_time(r, i) == cron[i].run_when_matched[j])
                        {
                            matches = 1;
                            break;
                        }
                    }
                    else{
                        matches = 1;
                    }
                }

                if (matches != 1){ //matches should always be equal to on each iteration through the different time constraints if the cronjob is supposed to run.
                    printf(1, "doesn't match on %s\n", labels[i]);
                    //printf(1, "time: %d", int_to_time(r, i));
                    execute = 0;
                    break;
                }

            }
            
            //if it does match, execute the command
            if (execute == 1){
                printf(1, "Job matches time criteria. Executing command\n\n");

                //execute commands
                if (fork() == 0){
                    exec(command->argv[0], command->argv);
                    exit();
                }
            }
        }


        //free stuff
        for (int i = 0; i < command->argc; i++)
        {
            free(command->argv[i]);
        }
        free(command->argv);

        
        struct Node *tmp;
        for (int i = 0; i < contents->count; i++)
        {
            printf(1, "freeing");
            tmp = contents->head;
            contents->head = contents->head->next;
            free(tmp->line);
            free(tmp);
        }


        iterations++;
        sleep(WAIT); //this will check the cron file every minute
    }
    
    exit();
}

void get_cron_details(Node *ln, cronarray *cron, struct command *command)
{

    //figures out how large to make param_array
    int param_size = 1; //starting at 1 because there will be 1 more param than spaces, since there isn't a space at the end.
    for (int i = 0; i < strlen(ln->line); i++)
    {
        if (ln->line[i] == ' ')
        {
            param_size++;
        }
    }


    char *param_array[param_size];
    int counter = 0; //int to place params in param_array
    int offset = 0;

    if (ln->line[0] != '#' && strlen(ln->line) > 3)
    {
        for (int i = 0; i <= strlen(ln->line); i++)
        {
            if (ln->line[i] == ' ' || ln->line[i] == '\0')
            {
                param_array[counter] = malloc((i - offset + 2) * sizeof(char)); //+ 2 because I may have to change an asterisk to a negative 1. and of course null terminator.
                memset(param_array[counter], 0, i - offset + 2); //clears old data
                memmove(param_array[counter], ln->line + offset, i - offset);
                param_array[counter][i - offset + 1] = '\0';
                counter++;
                offset = i + 1; //the plus 1 is to not include the extra space preceeding the symbols or numbers
            }
        }
        
        command->argv = malloc((param_size - 5 + 1) * sizeof(char**)); //+1 for the void pointer/null terminator
        command->argc = 0;
        offset = 0;
        for (int i = 0; i < (param_size - 5); i++)
        {
            command->argv[i] = param_array[i+5];
            command->argc++;
        }
        command->argv[command->argc] = NULL;//add the null terminator
    }
    

    //uncomment these to see the commands and params for testing.
    /*for (int k = 0; k < param_size; k++){
        printf(1,"'param: %s'\n", param_array[k]);
    }

    for (int k = 0; k < command->argc; k++){
        printf(1,"'command: %s'\n", command->argv[k]);
    }*/

    for (int i = 0; i < 5; i++) //loop through each param and convert it to an array of ints, or -1 if it is empty.
    {
        
        //printf(1, "%s\n", param_array[i]);


        if (strcmp(param_array[i], "*") == 0)
        {
            cron[i].run_when_matched[0] = -1;
            cron[i].size = 1;
        }   
        else
        {
            //splitting logic
            int mutli_valued = 0;
            for (int j = 0; j < strlen(param_array[i]); j++)
            {
                //printf(1, "getting here\n");
                if (param_array[i][j] == '-'){
                    //printf(1, "dash found\n");
                    mutli_valued = 1;
                    int start = 0;
                    int end = 0;
                    
                    char *initial = malloc((j + 1) * sizeof(char));
                    memset(initial, 0, j + 1);
                    memmove(initial, param_array[i], j);

                    char *final = malloc((strlen(param_array[i])-j) * sizeof(char));
                    strcpy(final, param_array[i]+(j+1));
                    

                    //printf(1, "initial: %s\n", initial);
                    //printf(1, "final: %s\n", final);


                    //handle day and month abbreviations
                    if ((i == 3 || i == 4) && is_digit(initial[0]) != 1)
                    {

                        if (i == 3)
                        {
                            start = month_to_int(initial);
                            end = month_to_int(final);
                            printf(1, "checking months. start: %d. end: %d", start, end);
                        }
                        else
                        {
                            start = day_to_int(initial);
                            end = day_to_int(final);
                            printf(1, "checking day_of_week. start: %d. end: %d", start, end);
                        }
                    }
                    else
                    {
                        start = atoi(initial);
                        end = atoi(final);
                    }

                    //printf(1, "start: %d. end: %d", start, end);


                    int l = 0;
                    for (int k = start; k <= end; k++){
                        cron[i].run_when_matched[l] = k;
                        l++;
                    }
                    cron[i].size = l;

                    free(final);
                    free(initial);

                }
            }

            if (mutli_valued == 0)
            {
                if ((i == 3 || i == 4) && (is_digit(param_array[i][0]) != 1))
                {

                    if (i == 3)
                    {
                        cron[i].run_when_matched[0] = month_to_int(param_array[i]);
                    }
                    else
                    {
                        cron[i].run_when_matched[0] = day_to_int(param_array[i]);
                    }
                }
                else
                {
                    cron[i].run_when_matched[0] = atoi(param_array[i]);
                }
                cron[i].size = 1;
            }
        }
    }

    //ree param array
    /*for(int i = 0; i < param_size; i++)
    {
        free(param_array[i]);
    }*/
    
    
}



int int_to_time(struct rtcdate r, int i){
    //memset(c, 0, 10 * sizeof(char));
    if (i == 0){
        return r.minute;
    }else if (i == 1){
        return r.hour;
    }else if (i == 2){
        return r.day;
    }else if (i == 3){
        return r.month;
    }else if (i == 4){
        //this should really calculate day of week. Not currently a top priority.
        return r.year;
    }else{
        return -1;
    }
}

int day_to_int(char *day)
{
    for (int i = 0; i < 7; i++)
    {
        if (strcmp(days[i], day) == 0)
        {
            return i;
        }
    }
    printf(1, "there is likely a problem in your day spelling");
    return -1;
}

int month_to_int(char *month)
{
    for (int i = 0; i < 12; i++)
    {
        if (strcmp(months[i], month) == 0)
        {
            return i + 1; //since months start at 1, not 0.
        }
    }

    printf(1, "there is likely a problem in your month spelling");
    return -1;
}

int is_digit(char testchar) //converted this answer to C (https://stackoverflow.com/a/26748560/5369588)
{
    for (int i = 0; i < strlen(NUMBERS); i++)
    {
        if (testchar == NUMBERS[i])
        {
            return 1;
        }
    }
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
        printf(1, "temp1: %s", temp);
        free(tempBuf);
        printf(1, "temp2: %s", temp);
        tempBuf = malloc(total * sizeof(char));
        
        if (temp[0] != 0)
        {

            memmove(tempBuf, temp, strlen(temp));

        }
        //Move contents into tempBuf

        memmove(tempBuf + (total - n), buf, n);

        free(temp);

    }
    //free(buf); //I'm not sure that I actually need to free this.    
    printf(1, "getting here 3");

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