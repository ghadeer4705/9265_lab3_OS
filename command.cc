
/*
 *
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>
#include "command.h"
#include "tokenizer.h"
static int log_fd = -1;

static bool isWord(TokenType t)
{
    return t == TOKEN_COMMAND || t == TOKEN_ARGUMENT;
}

static void destroySimple(SimpleCommand *s)
{
    for (int j = 0; j < s->_numberOfArguments; j++)
        free(s->_arguments[j]);
    free(s->_arguments);
    delete s;
}

static void setFile(char *&dst, const std::string &name)
{
    free(dst);
    dst = strdup(name.c_str());
}

static void redirect(const char *file, int flags, int target)
{
    int fd = open(file, flags, 0644);
    if (fd < 0)
    {
        perror(file);
        _exit(1);
    }
    dup2(fd, target);
    close(fd);
}
void parse(std::vector<Token> &tokens)
{
    Command cmd;
    SimpleCommand *simple = new SimpleCommand();
    const char *error = NULL;

    for (size_t i = 0; i < tokens.size() && !error; i++)
    {
        TokenType type = tokens[i].type;

        if (type == TOKEN_EOF)
            break;

        if (isWord(type))
        {
            simple->insertArgument(strdup(tokens[i].value.c_str()));
        }
        else if (type == TOKEN_PIPE)
        {
            if (simple->_numberOfArguments == 0)
                error = "Syntax error: missing command before pipe";
            else
            {
                cmd.insertSimpleCommand(simple);
                simple = new SimpleCommand();
            }
        }
        else if (type == TOKEN_BACKGROUND)
        {
            cmd._background = 1;
        }
        else
        {
            if (i + 1 >= tokens.size() || !isWord(tokens[i + 1].type))
            {
                error = "Syntax error: missing file name for redirection";
                break;
            }

            const std::string &file = tokens[i + 1].value;

            if (type == TOKEN_REDIRECT)
                setFile(cmd._outFile, file);
            else if (type == TOKEN_APPEND)
            {
                setFile(cmd._outFile, file);
                cmd._append = 1;
            }
            else if (type == TOKEN_INPUT)
                setFile(cmd._inputFile, file);
            else if (type == TOKEN_ERROR)
                setFile(cmd._errFile, file);
            else
            {
                setFile(cmd._outFile, file);
                setFile(cmd._errFile, file);
                cmd._out_error = 1;
                cmd._append = 1;
            }

            i++;
        }
    }

    bool empty = (simple->_numberOfArguments == 0);

    if (!error && empty && cmd._numberOfSimpleCommands > 0)
        error = "Syntax error: missing command after pipe";

    if (!error && empty && cmd._numberOfSimpleCommands == 0 &&
        (cmd._outFile || cmd._inputFile || cmd._errFile || cmd._background))
        error = "Syntax error: missing command";

    if (error || empty)
    {
        if (error)
            std::cout << error << std::endl;
        destroySimple(simple);
        cmd.clear();
        return;
    }

    cmd.insertSimpleCommand(simple);
    cmd.execute();

    /*
      Implement this function
      you must parse for all possible tokens scenario
      right now command.execute only print the format , you will inject it in your logic.

    */
}

SimpleCommand::SimpleCommand()
{
    _numberOfAvailableArguments = 5;
    _numberOfArguments = 0;
    _arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
    if (_numberOfAvailableArguments == _numberOfArguments + 1)
    {
        _numberOfAvailableArguments *= 2;
        _arguments = (char **)realloc(_arguments,
                                      _numberOfAvailableArguments * sizeof(char *));
    }

    _arguments[_numberOfArguments] = argument;
    _arguments[_numberOfArguments + 1] = NULL;

    _numberOfArguments++;
}

Command::Command()
{
    _numberOfAvailableSimpleCommands = 1;
    _simpleCommands = (SimpleCommand **)
        malloc(_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
    _out_error = 0;
    _append = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
    if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
    {
        _numberOfAvailableSimpleCommands *= 2;
        _simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
                                                    _numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
    }

    _simpleCommands[_numberOfSimpleCommands] = simpleCommand;
    _numberOfSimpleCommands++;
}

void Command::clear()
{
     for (int i = 0; i < _numberOfSimpleCommands; i++)
        destroySimple(_simpleCommands[i]);

    free(_outFile);
    free(_inputFile);
    free(_errFile);

    _numberOfSimpleCommands = 0;
    _outFile = NULL;
    _inputFile = NULL;
    _errFile = NULL;
    _background = 0;
    _out_error = 0;
    _append = 0;
}


void Command::print()
{
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        printf("  %-3d ", i);
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
            printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
        printf("\n");
    }

    printf("\n");
    printf("  Output       Input        Error        Err&Out      Background\n");
    printf("  ------------ ------------ ------------ ------------ ------------\n");
    printf("  %-12s %-12s %-12s %-12s %-12s\n",
           _outFile ? _outFile : "default",
           _inputFile ? _inputFile : "default",
           _errFile ? _errFile : "default",
           _out_error ? _errFile : "default",
           _background ? "YES" : "NO");
    printf("\n\n");
}

void Command::execute()
{
    if (_numberOfSimpleCommands == 0)
    {
        clear();
        return;
    }

    char *name = _simpleCommands[0]->_arguments[0];
    if (_numberOfSimpleCommands == 1 && strcmp(name, "exit") == 0)
    {
        printf("Good bye!!\n");
        clear();
        exit(0);
    }

       if (_numberOfSimpleCommands == 1 && strcmp(name, "cd") == 0)
    {
        char *path = _simpleCommands[0]->_arguments[1];
        if (path == NULL)
            path = getenv("HOME");

        if (path == NULL || chdir(path) != 0)
            perror("cd");

        clear();
        return;
    }

    int n = _numberOfSimpleCommands;
    int pipefd[2];
    int prev = -1;
    std::vector<pid_t> pids;
    sigset_t chld_set, old_mask;
    sigemptyset(&chld_set);
    sigaddset(&chld_set, SIGCHLD);
    sigprocmask(SIG_BLOCK, &chld_set, &old_mask);


    for (int i = 0; i < n; i++)
    {
        if (i < n - 1 && pipe(pipefd) < 0)
        {
            perror("pipe");
            break;
        }

        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork");
            break;
        }

        if (pid == 0)
        {
        sigprocmask(SIG_SETMASK, &old_mask, NULL);
        signal(SIGINT, SIG_DFL);

            if (prev != -1)
            {
                dup2(prev, STDIN_FILENO);
                close(prev);
            }

            if (i < n - 1)
            {
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }

            if (i == 0 && _inputFile)
                redirect(_inputFile, O_RDONLY, STDIN_FILENO);

            if (i == n - 1 && _outFile)
                redirect(_outFile, O_WRONLY | O_CREAT | (_append ? O_APPEND : O_TRUNC), STDOUT_FILENO);

            if (i == n - 1 && _errFile)
                redirect(_errFile, O_WRONLY | O_CREAT | (_out_error ? O_APPEND : O_TRUNC), STDERR_FILENO);

            execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);

            fprintf(stderr, "%s: command not found\n", _simpleCommands[i]->_arguments[0]);
            _exit(127);
        }

        pids.push_back(pid);

        if (prev != -1)
        {
            close(prev);
            prev = -1;
        }

        if (i < n - 1)
        {
            close(pipefd[1]);
            prev = pipefd[0];
        }
    }

    if (prev != -1)
        close(prev);
    if (!_background)
    {
        for (size_t k = 0; k < pids.size(); k++)
        {
            int status;
            pid_t result;

            do
            {
                result = waitpid(pids[k], &status, 0);
            }
            while (result < 0 && errno == EINTR);

            if (result > 0 && log_fd >= 0)
            {
                char msg[64];
                int len = snprintf(msg, sizeof(msg),
                                   "Child process %d terminated\n",
                                   (int)result);
                write(log_fd, msg, len);
            }
        }
    }

    sigprocmask(SIG_SETMASK, &old_mask, NULL);

    clear();
}

void Command::prompt()
{
    std::string input;

    while (true)
    {
        printf("myshell>");
        fflush(stdout);

        if (!std::getline(std::cin, input))
        {
            printf("\n");
            break;
        }

        std::vector<Token> tokens = tokenize(input);
        parse(tokens);
    }
}

void handle_sigchld(int signal_number)
{
    int saved_errno = errno;
    pid_t pid;

     while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        if (log_fd >= 0)
        {
            char msg[64];
            int len = snprintf(msg, sizeof(msg), "Child process %d terminated\n", (int)pid);
            ssize_t r = write(log_fd, msg, len);
            (void)r;
        }
    }
    errno = saved_errno;
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int main()
{
    log_fd = open("shell.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
    signal(SIGINT, SIG_IGN);
    signal(SIGCHLD, handle_sigchld);
    Command::_currentCommand.prompt();
    return 0;
}
