#ifndef MYSHELL_BUILTIN_H
#define MYSHELL_BUILTIN_H

#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <initializer_list>

namespace builtIn {

    int MERRNO_SUCCESS = 0;
    int MERRNO_FAILURE = -1;

    int merrno = 0;

    void print(char **);

//    char* getenvvar(char*, char **);

    bool checkHelpOption(char **, int);

    void handleMpwd(char **, int);

    void handleMcd(char **, int);

    void handleMexit(char **, int);

    void handleMecho(char **, int);

//    void handleMexport(char **, int, char **);
}


#endif //MYSHELL_BUILTIN_H
