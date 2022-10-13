#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

// Clearing the shell using escape sequences

//Read Input

//Prove what we have done and plan for next fortnight

int cmd_exec(char **pars);
void open_history();

char *readinp(void)
{
    int line = 1024;
    int pos = 0;
    char *allocchr = malloc(sizeof(char) * line);
    int c;
    while(1)
    {
        c = fgetc(stdin);
        if(c == '\n') //This sections checks whether we reached the end of the line
        {
            allocchr[pos] = '\0';
            return allocchr;
        }
        else
        {
            allocchr[pos] = c;
        }
        pos++;

        //Dynamic input code here
    }
}

char **tokenizeline(char *line)
{
    int linesize = 64;
    int pos = 0;
    char **tokens = malloc(linesize * sizeof(char*));
    char *token;

    //error allocation code here

    //Tokenizing bit
    token = strtok(line, " \t\n;&><");
    while (token != NULL)
    {
        tokens[pos] = token;
        pos++;

        //error allocation code here

        token = strtok(NULL, " \t\n;&><");
    }
    tokens[pos] = NULL;
    return tokens;

}

//Initate the shell

int init_shell()
{
    const char* clear_screen = " \e[1;1H\e[2J"; //make clear screen escape seq
    write (STDOUT_FILENO, clear_screen, 12); //Clear screen. Better than "clear()"?
    char* username = getenv("USER"); //getenv can be used for getting file path later with cd
    printf("\n\n\nUSER is: @%s", username);
    printf("\n");
    sleep(1); //is this really nessassary
    write (STDOUT_FILENO, clear_screen, 12);
}

//User input #

void usrinp(){
    char cwd[256];
    printf("%s-#",getcwd(cwd, sizeof(cwd)));
}

// HISTORY declarations
#define history_size 20
char** history_array[history_size] = {NULL};
int front = -1, rear = -1;

// saving command to history

void save_cmd(char **pars){
    if(rear != -1){
        int different = 0;
        char** element = history_array[rear];
        int i;
        for(i = 0; pars[i] != NULL && element[i] != NULL; i++){
            if(strcmp(pars[i], element[i]) != 0) different = 1;
        }
        if((!different) && (pars[i] == NULL && element[i] == NULL)) return;
    }
    char **pars_copy = malloc(64 * sizeof(char*));
    // copy pars
    for(int i = 0; pars[i] != NULL; i++){
        char* string = malloc(sizeof(char) * 1024);
        strcpy(string, pars[i]);
        pars_copy[i] = string;
    }
    // store in array
    if(front == -1){ // array empty
        front = 0;
        rear = 0;
        history_array[rear] = pars_copy;
        return;
    }
    rear = (rear + 1) % history_size;
    if(front == rear){
        front = (front + 1) % history_size;
    }
    free(history_array[rear]);
    history_array[rear] = pars_copy;
    return;
}

void history_invocation(char **pars){
    char* command = pars[0];
    if(command[1] == '!'){
        // latest command
        if(rear != -1) cmd_exec(history_array[rear]);
        else printf("Error: The history is empty\n");
        return;
    }
    if(command[1] == '-'){
        // nth from last
        int index = rear - (command[2] - '0');
        if(index >= 0 && index < history_size) cmd_exec(history_array[index]);
        else printf("Error: the given number is out of bounds!\n");
        return;
    }
    // nth command
    int index;
    if(front <= rear){
        index = command[1] - '0' - 1;
        if(index <= rear) cmd_exec(history_array[index]);
        else printf("Error: the given number is out of bounds!\n");
    }else{
        index = ((front + (command[1] - '0') - 1) % history_size);
        if(index <= rear) cmd_exec(history_array[index]);
        else printf("Error: the given number is out of bounds!\n");
    }
}

void save_history(){
    // open file
    FILE* f = fopen(".hist_list", "w");
    if(f == NULL){
        printf("Error: Cannot open history file!");
        return;
    }
    // write the front & rear values
    fprintf(f, "%d %d\n", front, rear);
    // write array elements
    int i;
    if(front <= rear){
        for(int j = front; j <= rear; j++){
            char** element = history_array[j];
            for(i = 0; element[i+1] != NULL; i++){
                fprintf(f, "%s ", element[i]);
            }
            fprintf(f, "%s", element[i]);
            fprintf(f, "\n");
        }
    }
    else{
        for(int j = front; j < history_size; j++){
            char** element = history_array[j];
            for(int i = 0; element[i] != NULL; i++){
                fprintf(f, "%s ", element[i]);
            }
            fprintf(f, "\n");
        }
        for(int j = 0; j <= rear; j++){
            char** element = history_array[j];
            for(int i = 0; element[i] != NULL; i++){
                fprintf(f, "%s ", element[i]);
            }
            fprintf(f, "\n");
        }
    }
    fclose(f);
}

void open_history(){
    // open file for reading
    FILE* f = fopen(".hist_list", "r");
    if(f == NULL){
        printf("Error: cannot open history file!");
        return;
    }

    // read front & rear
    fscanf(f, "%d %d\n", &front, &rear);
    if(front == -1) return;
    // read history
    if(front <= rear){
        for(int j = front; j <= rear; j++){
            char* line = malloc(sizeof(char) * 10);
            fscanf(f, "%s\n", line);
            //fgets(line, sizeof(line), f);
            char **par = tokenizeline(line);
            history_array[j] = par;
        }
    }
    else{
        for(int j = front; j < history_size; j++){
            char* line = malloc(sizeof(char) * 1024);
            //fscanf(f, "%s", line);
            fgets(line, sizeof(line), f);
            char **par = tokenizeline(line);
            history_array[j] = par;
        }
        for(int j = 0; j <= rear; j++){
            char* line = malloc(sizeof(char) * 1024);
            fscanf(f, "%s", line);
            char **par = tokenizeline(line);
            history_array[j] = par;
        }
    }

    fclose(f);
}

///////////////////////ALIASING/////////////////////////////////

// Alias declarations
char* altable [20][2][512] = {0}; //Dont question
char** aliastable[20];
char** aliasdef[20][512];


int list_alias(){
    for(int i = 0; i < 20; i++){
        if(aliastable[i] != NULL){
            printf("Alias Name: %s\n cmd: ", aliastable[i]);
            for (int j = 0; aliasdef[i][j] != NULL ; ++j) {
                printf("%s ", aliasdef[i][j]);
            }
        }
    }
    printf("\n");
    return 0;
}

void tokenReplacer(char **pars){
    char **cmd_copy = malloc(64 * sizeof(char*));
    int pos = 0;
    int flag = 0;
    char **tokens = malloc(64 * sizeof(char*));
    for(int i = 0; pars[i] != NULL; i++){
        char* string = malloc(sizeof(char) * 1024);
        strcpy(string, pars[i]);
        cmd_copy[i] = string;
    }
    for(int i = 0; i < sizeof(cmd_copy); i++){
        if (cmd_copy[i] != NULL) {
            for (int j = 0; j < 20; ++j) {
                if (aliastable[j] != NULL) {
                    if (strcmp(cmd_copy[i], aliastable[j]) == 0) {
                        printf("Token found\n");
                        for (int x = 0; aliasdef[j][x] != NULL; x++) {
                            char *string = malloc(sizeof(char) * 1024);
                            strcpy(string, aliasdef[j][x]);
                            tokens[pos] = string;
                            pos++;
                        }
                        flag = 1;
                        break;
                    }
                }
            }
        }
    }
    if (flag == 1) {
        cmd_exec(tokens);
        free(cmd_copy);
        free(tokens);
    }
}

void saveAlias(){
    FILE* f = fopen(".Alias_list", "w");
    if(f == NULL){
        printf("Error: Cannot open Alias file!");
        return;
    }
    for(int i = 0; i < 20; i++){
        if (aliastable[i] != NULL) {
            char *alias = aliastable[i];
            fprintf(f, "%s\n", alias);
            for (int j = 0;j<512; ++j) {
                if (aliasdef[i][j] != NULL) {
                    char *cmd = aliasdef[i][j];
                    fprintf(f, "%s ", cmd);
                }
                else
                {
                    fprintf(f, "%s", "\n");
                    break;
                }
            }
        }
    }
    fclose(f);
}

void loadAlias(){
    char* alias;
    int i = 0;
    FILE* f = fopen(".Alias_list", "r");
    if(f == NULL){
        printf("Error: cannot open history file!");
        return;
    }
    while(fscanf(f, "%s ", alias) != EOF){
        do {

        } while (0);
    }
}

///////////////////////FORKING//////////////////////////////////
int forking(char **pars)
{
    int flag;
    pid_t cpid, apid;
    cpid = fork();
    if (cpid == 0)
    {
        if(execvp(pars[0], pars) == -1) { ;
            perror("Error");
        }
        exit(EXIT_FAILURE);
    }
    else if(cpid < 0){
        perror("Error");
    }
    else{
        do {
            apid = waitpid(cpid, &flag, WUNTRACED);
        } while (!WIFEXITED(flag) && !WIFSIGNALED(flag));
    }
    return 0;
}

//////////////////////BUILT IN COMMAND DECLARATIONS & COMMAND EXECUTE FUNC///////////////////////////
int cexit(char **pars);
int getpath(char **pars);
int setpath(char **pars);
int cd(char **pars);
int history(char **pars);
int alias(char **pars);
int unalias(char **pars);
int list_alias();
char *builtinstrings[] = {"exit","getpath","setpath","cd", "history", "alias", "unalias", "list_alias"};
int (*builtinfunc[]) (char**) = {&cexit, &getpath, &setpath, &cd, &history, &alias, &unalias, &list_alias};

int cmd_exec(char **pars)
{
    tokenReplacer(pars);
    if (pars[0] == NULL)
    {
        return 0;
    }

    // check for history invocations
    if(pars[0][0] == '!'){
        // handle history invocations
        history_invocation(pars);
        return 0;
    }
    else{
        save_cmd(pars);
    }

    for (int i = 0; i < 8; ++i) {
        if(strcmp(pars[0], builtinstrings[i]) == 0)
        {
            return (*builtinfunc[i])(pars);
        }
    }
    return forking(pars);
}

//////////////////////COMMAND DEFINITIONS/////////////////////////////////////
int unalias(char **pars){
    for(int i = 0; i < 20; i++){
        if (aliastable[i] != NULL)
        {
            if (strcmp(aliastable[i], pars[1]) == 0)
            {
                aliastable[i] = NULL;
                for (int j = 0; j < 512; ++j) {
                    if (aliasdef[i][j] != NULL) {
                        aliasdef[i][j] = NULL;
                    }
                    break;
                }
                printf("Alias removed\n");
            }
        }
    }
    return 0;
}

int alias(char **pars){

    char **pars_copy = malloc(64 * sizeof(char*));
    // copy pars
    for(int i = 0; pars[i] != NULL; i++){
        char* string = malloc(sizeof(char) * 1024);
        strcpy(string, pars[i]);
        pars_copy[i] = string;
    }

    int emptypos = -1;
    if(pars_copy[2] == NULL){
        printf("lack of arguements\n");
        return 0;
    }
    for (int i = 0; i < 20; ++i) { //Checks for alias already existing and an empty position in the table
        if (aliastable[i] != NULL)
        {
            if(strcmp(pars_copy[1], aliastable[i]) == 0) {
                printf("Alias already exists\n");
                return 0;
            }
        }
        else
        {
            emptypos = i;
        }
    }
    if (emptypos > -1)
    {
        aliastable[emptypos] = pars_copy[1];
        for (int i = 0; i < sizeof(pars_copy); ++i) {
            for (int j = 0; j < 512; ++j) {
                if (i > 1 && aliasdef[emptypos][j] == NULL)
                {
                    aliasdef[emptypos][j] = pars_copy[i];
                    break;
                }
            }
        }
    }
    free(pars_copy);
    return 0;
}

int cexit(char **pars)
{
    saveAlias();
    save_history();
    return 1;
}

int getpath(char **pars)
{
    printf("%s\n",getenv("PATH"));
    return 0;
}

int setpath(char **pars)
{
    if (pars[1] == NULL)
    {
        printf("Error expected arguments");
    }
    else
    {
        if (setenv("PATH",pars[1],1) == 0)
        {
            printf("Successful\n");
        }
        else
        {
            printf("Error\n");
        }
    }
    return 0;
}

int cd(char **pars)
{
    if (pars[1] == NULL)
    {
        chdir(getenv("HOME"));
    }
    else
    {
        if (chdir(pars[1]) != 0)
        {
            perror("\nError:");
            return 0;
        }
    }
    return 0;
}

int history(char **pars){
    // print the history
    if(front <= rear){
        for(int j = front; j <= rear; j++){
            printf("%d ", j - front + 1);
            char** element = history_array[j];
            for(int i = 0; element[i] != NULL; i++){
                printf("%s ", element[i]);
            }
            printf("\n");
        }
    }
    else{
        for(int j = front; j < history_size; j++){
            printf("%d ", j - front + 1);
            char** element = history_array[j];
            for(int i = 0; element[i] != NULL; i++){
                printf("%s ", element[i]);
            }
            printf("\n");
        }
        for(int j = 0; j <= rear; j++){
            printf("%d ", (history_size - front) + j + 1);
            char** element = history_array[j];
            for(int i = 0; element[i] != NULL; i++){
                printf("%s ", element[i]);
            }
            printf("\n");
        }
    }
    return 0;
}

int main()
{
    //////////////////////DECLARATIONS/////////////////////////
    int flag = 0;
    char *line;
    char **par;

    /////////////////MAIN SHELL/////////////////////
    init_shell(); //initiates our shell
    char orgpath = *getenv("PATH");
    chdir(getenv("HOME"));
    open_history();
    loadAlias();
    do {
        usrinp();
        line = readinp();
        par = tokenizeline(line);
        flag = cmd_exec(par);
        free(line);
        free(par);
    } while (flag == 0);
    for(int i = 0; i < history_size; i++) free(history_array[i]);
    return flag;
}
