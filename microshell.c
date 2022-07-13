#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void show(char *str)
{
    int i = 0;
    while (str[i])
        write(2, &str[i++], 1);
}

void cd(char **cmd)
{
    if (cmd[2])
        show("error: cd: bad arguments");
    else
    {
        if (chdir(cmd[1]) < 0)
        {
            show("error: cd: cannot change directory to ");
            show(cmd[1]);
            show("\n");
        }
    }
}

int main(int ac, char **str, char **env)
{
    int i = 1;
    char *cmd[1000000] = {NULL};
    int fd[2];
    int pip = 0;
    int j;
    int pid;
    int n  = 0;
    if (ac == 1)
        return 0;
    while (str[i])
    {
        pip = 0;
        j = 0;
        while (str[i] && (!strcmp(str[i], "|") || !strcmp(str[i], ";")))
            i++;
        if (str[i] == NULL)
            break;
        while (str[i] && strcmp(str[i], "|") && strcmp(str[i], ";"))
            cmd[j++] = str[i++];
        cmd[j] = NULL;
        if (!strcmp("cd", cmd[0]))
            cd(cmd);
        else
        {
            if (str[i] && !strcmp("|", str[i]))
            {
                pip = 1;
                if (pipe(fd) < 0)
                {
                    show("error: fatal\n");
                    exit(1);
                }
            }
            if ((pid = fork()) < 0)
            {
                show("error: fatal\n");
                exit(1);
            }
            n++;
            if (!pid)
            {
                if (pip)
                {
                    close(1);
                    dup2(fd[1], 1);
                    close(fd[0]); close(fd[1]);
                }
                if (execve(cmd[0], cmd, env) < 0)
                {
                    show("error: cannot execute ");
                    show(cmd[0]);
                    show("\n");
                    exit(1);
                }
            }
            if (!pip)
            {
                for(int i = 0; i < n; i++)
                    waitpid(-1, NULL, 0);
                n  = 0;
            }
            if (pip)
            {
                close(0);
                dup2(fd[0], 0);
                close(fd[1]); close(fd[0]);
            }
        }
        if (!str[i])
            break;
        i++;
    }
}