/*
Old Linked List Functions
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
void insert(Node *ln, List *contents, int line_num)
{

    for (int i = 0; i < line_num; i++)
    {
    }
    ln->prev = contents->end;
    contents->end->next = ln;
    contents->end = ln;
    contents->count++;
}

void append(Node *ln, List *contents)
{
    //ln->prev = contents->end;
    contents->end->next = ln;
    contents->end = ln;
    contents->count++;
}

    
    for (Node *ln = contents->head; ln != 0; ln = ln->next)
    {
        if (i >= (converted_range->start - 1) && i <= converted_range->end)
        { //see (https://www.codesdope.com/blog/article/deletion-of-a-give-node-from-a-linked-list-in-c/#:~:text=We%20delete%20any%20node%20of,'%20i.e.%2C%20a%20%E2%86%92%20c.)
           Node *temp;
           temp = ln->next;
           ln->next = temp->next;
           free(temp);
        }
        i++;
    }
    //list("test", contents);
    

               for (int k = 0; k < strlen(text); k++) //loops through every char in the text being searched for
            {
                if (text[k] == ln->line[j])
                {

                }
            }

    for (int i = 0; i < strlen(command->argv[0]); i++)
    {
        for (int j = 0; j < 13; j++)
        {
            if (commands_alone[j][i] != command->argv[0][i])
            {
                temp_list[j][0] = '\0';
            }
        }
    }
*/