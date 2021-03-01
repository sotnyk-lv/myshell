#ifndef MYSHELL_SHELL_H
#define MYSHELL_SHELL_H

#include <cstddef>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <fcntl.h>

#define MAXCOM 1024 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported

class shell {

public:

    shell();

    ~shell();

    // read line stored in buff;
    // write the result of the execution in the buff;
    int execLine(char *buff);

    // puts current directory of the shell in buffer
    static int getPwd(char *buff);

private:

    char* piped_commands[MAXLIST]{};
    pid_t pids[MAXLIST]{};
    int pipes[MAXLIST][2];
    static const int NoOfOwnCmds = 5;

    const char* ListOfOwnCmds[NoOfOwnCmds] = {"merrno",
                                              "mecho",
                                              "mpwd",
                                              "mexit",
                                              "mcd"};

    int parsePipes(char *buff);

    int execPipes(char *buff, int number_of_piped_commands);

    static void ch_stdin(int*);
    static void ch_stdout(int*);
    void close_pipes(int* pipefd);

    int execSingleCommand(char* command);

    static void executeArgs(char** parsed);

    int executeBuiltInComs(char** parsed, int size);

    static int parseSpace(char* str, char** parsed);

    static bool isInternalScript(char *inputString);

    void runInternalScript(char* inputString);

    static bool isExternalScript(char *inputString);
};

#endif //MYSHELL_SHELL_H
