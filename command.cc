
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
#include <time.h>
#include <glob.h>
#include <wordexp.h>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include "command.h"
#include "tokenizer.h"
#include <errno.h>

void parse(std::vector<Token> &tokens)
{
    Command currentCommand;
    SimpleCommand *currentSimpleCommand = new SimpleCommand();
 for (size_t i = 0; i < tokens.size(); i++)
    {
        if (tokens[i].type == TOKEN_EOF)
        {
            break;
        }

        if (tokens[i].type == TOKEN_COMMAND ||
            tokens[i].type == TOKEN_ARGUMENT)
        {
            char *argument = strdup(tokens[i].value.c_str());
            currentSimpleCommand->insertArgument(argument);
        }
        else if (tokens[i].type == TOKEN_PIPE)
        {
 if (currentSimpleCommand->_numberOfArguments == 0)
    {
        std::cout << "Syntax error: missing command before pipe" << std::endl;
        delete currentSimpleCommand;
        currentCommand.clear();
        return;
    }

            currentCommand.insertSimpleCommand(currentSimpleCommand);
            currentSimpleCommand = new SimpleCommand();
        }
        else if (tokens[i].type == TOKEN_REDIRECT ||
                 tokens[i].type == TOKEN_APPEND ||
                 tokens[i].type == TOKEN_INPUT ||
                 tokens[i].type == TOKEN_ERROR ||
                 tokens[i].type == TOKEN_REDIRECT_AND_ERROR)
        {
            if (i + 1 >= tokens.size() ||
                tokens[i + 1].type == TOKEN_EOF)
            {
                std::cout << "Missing filename for redirection" << std::endl;
                delete currentSimpleCommand;
                currentCommand.clear();
                return;
            }

            char *filename = strdup(tokens[i + 1].value.c_str());

            if (tokens[i].type == TOKEN_REDIRECT)
                currentCommand._outFile = filename;
            else if (tokens[i].type == TOKEN_APPEND)
            {
                currentCommand._outFile = filename;
                currentCommand._append = 1;
            }
            else if (tokens[i].type == TOKEN_INPUT)
                currentCommand._inputFile = filename;
            else if (tokens[i].type == TOKEN_ERROR)
                currentCommand._errFile = filename;
            else
            {
                currentCommand._outFile = strdup(tokens[i + 1].value.c_str());
                currentCommand._errFile = filename;
                currentCommand._out_error = 1;
                currentCommand._append = 1;
            }

            i++;
        }
        else if (tokens[i].type == TOKEN_BACKGROUND)
        {
            currentCommand._background = 1;
        }
    }
if (tokens.size() > 2 &&
    tokens[tokens.size() - 2].type == TOKEN_PIPE)
{
    std::cout << "Syntax error: missing command after pipe" << std::endl;
    delete currentSimpleCommand;
    currentCommand.clear();
    return;
}
    if (currentSimpleCommand->_numberOfArguments > 0)
    {
        currentCommand.insertSimpleCommand(currentSimpleCommand);
    }
    else
    {
        delete currentSimpleCommand;
    }

    currentCommand.execute();
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
    {
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            free(_simpleCommands[i]->_arguments[j]);
        }

        free(_simpleCommands[i]->_arguments);
        free(_simpleCommands[i]);
    }

    if (_outFile)
    {
        free(_outFile);
    }

    if (_inputFile)
    {
        free(_inputFile);
    }

    if (_errFile)
    {
        free(_errFile);
    }

    
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
        {
            printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
        }
    }

    printf("\n\n");
    printf("  Output       Input        Error        Err&Out       Background\n");
    printf("  ------------ ------------ ------------ ------------ ------------\n");
    printf("  %-12s %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
           _inputFile ? _inputFile : "default", _errFile ? _errFile : "default", _out_error == 1 ? _errFile : "default"
           ,_background ? "YES" : "NO");
    printf("\n\n");
}

void Command::execute()
{
    print();

    if (_numberOfSimpleCommands == 1 &&
        strcmp(_simpleCommands[0]->_arguments[0], "exit") == 0)
    {
        printf("Good bye!!\n");
        clear();
        exit(0);
    }

    if (_numberOfSimpleCommands == 1 &&
        strcmp(_simpleCommands[0]->_arguments[0], "cd") == 0)
    {
        char *path = _simpleCommands[0]->_arguments[1];

        if (path == NULL)
        {
            path = getenv("HOME");
        }

        if (chdir(path) != 0)
        {
            perror("cd");
        }

        clear();
        return;
    }
    
    if (_numberOfSimpleCommands == 0)
    {
        clear();
        return;
    }
  

    int pipefd[2];
    int previous_input = -1;
    pid_t pids[100];
    int number_of_pids = 0;

    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        if (i < _numberOfSimpleCommands - 1)
        {
            if (pipe(pipefd) < 0)
            {
                perror("pipe");
                clear();
                return;
            }
        }

        pid_t pid = fork();

        if (pid == 0)
        {
	signal(SIGINT, SIG_DFL);
            if (previous_input != -1)
            {
                dup2(previous_input, STDIN_FILENO);
                close(previous_input);
            }

            if (i < _numberOfSimpleCommands - 1)
            {
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }

            if (i == 0 && _inputFile != NULL)
            {
                int fd = open(_inputFile, O_RDONLY);
                if (fd < 0)
                {
                    perror("open");
                    _exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            if (i == _numberOfSimpleCommands - 1 && _outFile != NULL)
            {
                int flags = O_WRONLY | O_CREAT;
                flags |= _append ? O_APPEND : O_TRUNC;

                int fd = open(_outFile, flags, 0644);
                if (fd < 0)
                {
                    perror("open");
                    _exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            if (i == _numberOfSimpleCommands - 1 && _errFile != NULL)
            {
                int flags = O_WRONLY | O_CREAT;
                flags |= (_out_error || _append) ? O_APPEND : O_TRUNC;

                int fd = open(_errFile, flags, 0644);
                if (fd < 0)
                {
                    perror("open");
                    _exit(1);
                }
                dup2(fd, STDERR_FILENO);
                close(fd);
            }

            execvp(_simpleCommands[i]->_arguments[0],
                   _simpleCommands[i]->_arguments);

            perror("execvp");
            _exit(1);
        }
        else if (pid > 0)
        {
            pids[number_of_pids++] = pid;
        }
        else
        {
            perror("fork");
            clear();
            return;
        }

        if (previous_input != -1)
        {
            close(previous_input);
        }

        if (i < _numberOfSimpleCommands - 1)
        {
            close(pipefd[1]);
            previous_input = pipefd[0];
        }
    }

    if (previous_input != -1)
    {
        close(previous_input);
    }

    if (!_background)
    {
        for (int i = 0; i < number_of_pids; i++)
        {
            waitpid(pids[i], NULL, 0);
        }
    }

    clear();

}


void Command::prompt()
{
    std::string input;

    while (true)
    {
        printf("myshell>");
        fflush(stdout);

        std::getline(std::cin, input);
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
        const char message[] = "Child process terminated\n";
        write(STDOUT_FILENO, message, sizeof(message) - 1);
    }

    errno = saved_errno;
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int main()
{
    signal(SIGINT, SIG_IGN);
    signal(SIGCHLD, handle_sigchld);
    Command::_currentCommand.prompt();
    return 0;
}
