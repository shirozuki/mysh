#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <limits.h>

#define BUFSIZE 1024
#define MAX_ARGS 128
#define SEPARATORS " \t\n"
#define TOKEN_MAX_SIZE 128
#define CWD_NAME_MAX 1024

void print_prompt()
{
    char host[HOST_NAME_MAX];
    char cwd[CWD_NAME_MAX];
    char *username;
    username = getlogin();
    getcwd(cwd, sizeof(cwd));
    gethostname(host, HOST_NAME_MAX);
    printf("[%s@%s %s]$ ", username, host, cwd);
}

int builtin_cd(char **args)
{
    if(args[1] == NULL)
    {
        fprintf(stderr, "cd: nie podano ścieżki!\n");
        return 1;
    }
    else
    {
        if(chdir(args[1]) != 0)
        {
            fprintf(stderr, "cd: podana ścieżka nie istnieje!\n");
            return 1;
        }
    }
}

void builtin_exit(char **args)
{
    if(args[1] == NULL)
    {
        exit(EXIT_SUCCESS);
    }
    else
    {   if((atoi(args[1]) >= 0) && (atoi(args[1]) <= 255))
            exit(atoi(args[1]));
        else
            fprintf(stderr, "exit: kod powrotu musi mieścić się w zakresie 0 - 255!\n");
    }
}

void builtin_version()
{
    int i;
    for(i = 0; i <= 7; i++)
        printf("#-#");
    printf("\n\'mysh\'\n");
    printf("Artur Dobrowolski\n");
    for(i = 0; i <= 7; i++)
        printf("#-#");
    printf("\n");
}

void builtin_help()
{
    int i;
    for(i = 0; i <= 23; i++)
        printf("#-#");
    printf("\nPodręcznik powłoki \'mysh\'\n\n"
           "Powłoka obsługuje uruchamianie programów znajdujących się w ścieżce PATH.\n"
           "Powłoka może uruchomić inny plik wykonywalny po wskazaniu do niego ścieżki\n"
           "względnej lub bezwzlgędnej.\n\n"

           "Lista poleceń wbudowanych powłoki \'mysh\':\n\n"
           "cd:   zmiana katalogu bieżacego,\n"
           "      składnia polecenia \'cd ŚCIEŻKA\'\n\n"

           "exit: wyjście z powłoki,\n"
           "      składnia polecenia \'exit KOD_POWROTU\'\n\n"

           "echo: wypisanie linii tekstu do standardowego wyjścia,\n"
           "      składnia polecenia \'echo STRING\'\n"
           "      echo może wypisać wartość zmiennej środowiskowej,\n"
           "      gdy nazwę zmiennej poprzedzi się znakiem \'$\'.\n\n"

           "kill: wysyła sygnał SIGTERM do procesu,\n"
           "      składnia polecenia \'kill PID\'.\n\n"

           "pwd:  wypisuje aktualną ścieżkę.\n\n");

    for(i = 0; i <= 23; i++)
        printf("#-#");
    printf("\n");
}

void builtin_echo(char **args)
{
    int i;
    for(i = 1; args[i] != NULL; i++)
    {
        char *string = args[i];
        if(*(string + 0) == '$')
        {
            char *var;
            var = getenv(string + 1);
            printf("%s ", var);
        }
        else
            printf("%s ", args[i]);
    }
    printf("\n");
}

int builtin_kill(char **args)
{
    int i;
    pid_t pid;
    for(i = 0; args[i] != NULL; i++);
    if(i < 2)
    {
        printf("Błędne polecenie!\n");
        printf("Proszę spróbować: 'kill PID'\n");
        return 1;
    }
    else
    {
        pid = atoi(args[1]);
        if(pid > 0 && pid < 32768)
        {
            int signal = kill(pid, SIGKILL);
            if(signal == -1)
            {
                    perror("kill");
                    return 1;
            }
        }
        else
        {
            printf("Błędne polecenie!\n");
            printf("PID musi być wartością liczbową!\n");
        }
    }
    return 0;
}

void builtin_pwd()
{
    char pwd[CWD_NAME_MAX];
    getcwd(pwd, sizeof(pwd));
    puts(pwd);
}

char *read_line(void)
{
    char *command = NULL;
    ssize_t bufsize = 0;
    getline(&command, &bufsize, stdin);
    return command;
}

char **splitter(char *string)
{
    int i = 0;
    char buffer[BUFSIZ];
    strcpy(buffer, string);
    char **array = malloc(BUFSIZ * sizeof(char));
    char *p = strtok(buffer, SEPARATORS);
    while(p != NULL)
    {
        array[i++] = p;
        p = strtok(NULL, SEPARATORS);
    }

    return array;
}

int execute(char **args)
{
    if(args[0])
    {
        if((strcmp(args[0], "cd")) == 0)
        {
            builtin_cd(args);
        }
        else if((strcmp(args[0], "exit")) == 0)
        {
            builtin_exit(args);
        }
        else if((strcmp(args[0], "echo")) == 0)
        {
            builtin_echo(args);
        }
        else if((strcmp(args[0], "kill")) == 0)
        {
            builtin_kill(args);
        }
        else if((strcmp(args[0], "pwd")) == 0)
        {
            builtin_pwd();
        }
        else if((strcmp(args[0], "help")) == 0)
        {
            builtin_help();
        }
        else if((strcmp(args[0], "version")) == 0)
        {
            builtin_version();
        }
        else
        {
            pid_t child = fork();
            if(child == 0)
            {
                execvp(args[0], args);
                fprintf(stderr, "Nieznane polecenie!\n");
                exit(EXIT_SUCCESS);
            }
            else
            {
                wait(NULL);
            }
        }
    }
    return 0;
}

void main_loop()
{
    char *command;
    char **tokens;
    int signal;
    while(1)
    {
        print_prompt();
        command = read_line();
        if(*command == '\n' || *command == '\t' || *command == ' ')
        {
            continue;
        }
        tokens = splitter(command);
        signal = execute(tokens);
    }
}

int main(int argc, char *argv)
{
    main_loop();
    return 0;
}
