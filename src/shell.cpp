#include "shell/shell.h"
#include "builtIn/builtIn.h"

shell::shell() = default;

shell::~shell() = default;

int shell::execLine(char *buff) {

    if (strlen(buff) == 0) {
        printf("> input: empty line");
        getPwd(buff);
        return 0;
    }

    uint number_of_piped_processes = parsePipes(buff);
    if (number_of_piped_processes > MAXLIST) {
        printf("> error: number of pipes overflow\n");
        return -1;
    }

    if (execPipes(buff, number_of_piped_processes) < 0) {
        printf("  > error while executing command\n");
    }
    getPwd(buff);

    return 0;
}

int shell::parsePipes(char *buff) {

    char *temp_buff = new char[1024];
    strcpy(temp_buff, buff);

    uint i=0;
    while ((piped_commands[i]  = strsep(&temp_buff, "|")) != nullptr) {
        ++i;
    }
    bzero(buff, strlen(buff));
    return i;
}

int shell::execPipes(char *buff, int number_of_piped_commands) {

    if (number_of_piped_commands == 1) {
        // if command is internal - shouldn't fork
        for (int i=0; i < 3; ++i) {
            if (strncmp(piped_commands[0], ListOfOwnCmds[i], strlen(ListOfOwnCmds[i])) == 0) {
                int saved_stdout = dup(STDOUT_FILENO);
                int out_pipe[2];
                if( pipe(out_pipe) != 0 ) {
                    printf("> error: can't create pipe for buffer out\n");
                    return -1;
                }
                dup2(out_pipe[1], STDOUT_FILENO);
                close(out_pipe[1]);
                execSingleCommand(piped_commands[0]);
                fflush(stdout);

                read(out_pipe[0], buff, MAXCOM);
                dup2(saved_stdout, STDOUT_FILENO);
                close(out_pipe[0]);

                return 0;
            }
        }
        for (int i=3; i < NoOfOwnCmds; ++i) {
            if (strncmp(piped_commands[0], ListOfOwnCmds[i], strlen(ListOfOwnCmds[i])) == 0) {

                execSingleCommand(piped_commands[0]);

                return 0;
            }
        }

        // external single comand - fork

        // create pipe to cache output and read to buffer

        int out_pipe[2];
        if( pipe(out_pipe) != 0 ) {
            printf("> error: can't create pipe for buffer out\n");
            return -1;
        }

        pid_t pid = fork();
        if (pid < 0) {
            printf("> error: could not fork\n");
            return -1;
        }
        if (pid == 0) {
            ch_stdout(out_pipe);
            execSingleCommand(piped_commands[0]);

            exit(0);
        }

        wait(nullptr);
        close(out_pipe[1]);
        read(out_pipe[0], buff, MAXCOM);
        read(out_pipe[0], buff + strlen(buff), MAXCOM - strlen(buff));
        close(out_pipe[0]);

        return 0;
    }

    // piped commands

//    // pipe for input
//    int pipe_fd[2];
//    // pipe for output
//    int new_pipefd[2];

    if (pipe(pipes[0]) < 0) {
        printf("\n> error: pipe could not be initialized");
        return -1;
    }

    pids[0] = fork();
    if (pids[0] < 0) {
        printf("\n> error: could not fork");
        return -1;
    }
    else if (pids[0] == 0) {
        ch_stdout(pipes[0]);
        execSingleCommand(piped_commands[0]);
        exit(0);
    }
    wait(nullptr);
    printf("  > 0 fork done\n");

    // v _______________ for middle pipes _______________ v
    for(int i = 1; i < number_of_piped_commands - 1; ++i) {
        if (pipe(pipes[i]) < 0) {
            printf("\n> error: pipe could not be initialized");
            return -1;
        }

        pids[i] = fork();
        if (pids[i] < 0) {
            printf("\n> error: could not fork");
            return -1;
        } else if (pids[i] == 0) {
            ch_stdin(pipes[i-1]);
            ch_stdout(pipes[i]);
            execSingleCommand(piped_commands[i]);
            exit(0);
        }
        close_pipes(pipes[i-1]);
        wait(nullptr);
        printf("  > %d fork done\n", i);
    }
    // ^ _______________ for middle pipes _______________ ^

    int out_pipe[2];
    if( pipe(out_pipe) != 0 ) {
        printf(">\n error: can't create pipe for buffer out");
        return -1;
    }

    pids[number_of_piped_commands-1] = fork();
    if (pids[number_of_piped_commands-1] < 0) {
        printf("\n> error: could not fork");
        return -1;
    } else if (pids[number_of_piped_commands-1] == 0) {
        ch_stdin(pipes[number_of_piped_commands-2]);
        ch_stdout(out_pipe);
//        close_pipes(pipes[number_of_piped_commands-2]);
        execSingleCommand(piped_commands[number_of_piped_commands - 1]);
        exit(0);
    }
    printf("> reading\n");
    close_pipes(pipes[number_of_piped_commands-2]);

    printf("  > %d fork done\n", number_of_piped_commands-1);

    close(out_pipe[1]);
    wait(nullptr);
    read(out_pipe[0], buff, MAXCOM);
    close(out_pipe[0]);
    printf("> after read\n");
    return 0;
}

void shell::ch_stdin(int* pipefd) {
    close(pipefd[1]);
    dup2(pipefd[0], STDIN_FILENO);
    close(pipefd[0]);
}
void shell::ch_stdout(int* pipefd) {
    close(pipefd[0]);
    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[1]);
}

void shell::close_pipes(int* pipefd) {
    close(pipefd[0]);
    close(pipefd[1]);
}

int shell::execSingleCommand(char* command) {

    char  *parsed[MAXLIST];

    if (isInternalScript(command)) {
        runInternalScript(command);
        return 0;
    }

    if (isExternalScript(command)) {
        char *myshell = (char*)"./myshell ";
        char input[strlen(myshell) + strlen(command)];
        int i = 0;
        while (i < strlen(myshell)) {
            input[i] = myshell[i];
            ++i;
        }
        while (i - strlen(myshell) < strlen(command)) {
            input[i] = command[i - strlen(myshell)];
            ++i;
        }
        input[i] = '\0';
        execSingleCommand(input);
        return 0;
    }

    auto size = parseSpace(command, parsed);
    int builtInFound = executeBuiltInComs(parsed, size);
    if (!builtInFound) {
        executeArgs(parsed);
    }
    return 0;
}


// function for parsing command words
int shell::parseSpace(char* str, char** parsed)
{
    int i;
    for (i = 0; i < MAXLIST; i++) {
        parsed[i] = strsep(&str, " ");

        if (parsed[i] == nullptr) {
            return i;
        }
        if (strlen(parsed[i]) == 0)
            i--;
    }
    return MAXLIST;
}

int shell::executeBuiltInComs(char** parsed, int size)
{
    int i, switchOwnArg = 0;

    for (i = 0; i < NoOfOwnCmds; i++) {
        if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) {
            switchOwnArg = i + 1;
            break;
        }
    }
    switch (switchOwnArg) {
        case 1:
            std::cout  << builtIn::merrno ;
            return 1;
        case 2:
            builtIn::handleMecho(parsed, size);
            return 1;
        case 3:
            builtIn::handleMpwd(parsed, size);
            return 1;
        case 4:
            builtIn::handleMexit(parsed, size);
        case 5:
            builtIn::handleMcd(parsed, size);
            return 1;

        default:
            break;
    }

    return 0;
}

void shell::executeArgs(char** parsed)
{

    if (execvp(parsed[0], parsed) < 0) {
        printf("\nCould not execute command.. %s",parsed[0]);
    }

}

bool shell::isInternalScript(char *inputString) {
    // remove spaces before
    int i = 0;
    while (inputString[i] == ' ') ++i;
    memmove(inputString, inputString + i, strlen(inputString) - i);
    inputString[strlen(inputString) - i] = '\0';

    int size  = strlen(inputString);
    return size > 6 && inputString[0] == '.'  && inputString[size - 3] == 'm'
    && inputString[size - 2] == 's' && inputString[size - 1] == 'h' && inputString[size - 4] == '.';

}

bool shell::isExternalScript(char *inputString) {

    // remove spaces before
    int i = 0;
    while (inputString[i] == ' ') ++i;
    memmove(inputString, inputString + i, strlen(inputString) - i);
    inputString[strlen(inputString) - i] = '\0';

    int size  = strlen(inputString);
    return size > 6 && inputString[0] == '.' && inputString[1] == '/' && inputString[size - 4] == '.'
           && inputString[size - 3] == 'm' && inputString[size - 2] == 's' && inputString[size - 1] == 'h';
}

void shell::runInternalScript(char* inputString) {

    std::fstream new_file;
    new_file.open(inputString, std::ios::in);
    if (new_file.is_open()){
        std::string tp;
        while(getline(new_file, tp)){
            auto input = static_cast<char *>(malloc(strlen(tp.c_str()) + 1));
            strcpy(input, tp.c_str());
            execSingleCommand(input);
            free(input);
        }
        new_file.close();
        builtIn::merrno = builtIn::MERRNO_SUCCESS;
        return;
    }
    builtIn::merrno = builtIn::MERRNO_FAILURE;
}


int shell::getPwd(char *buff) {
    strcpy(buff + strlen(buff), ("\n" + std::filesystem::current_path().string() + "$ ").c_str());
    return 0;
}