#ifndef ADDER_IOPOSIX_H
#define ADDER_IOPOSIX_H

#include <unistd.h>
#include <cerrno>

namespace ioposix {

//    constexpr int FD_STDIN  = 0;
    constexpr int FD_STDOUT = 1;
//    constexpr int FD_STDERR = 2;


    int writeBuffer(int, const char *, int, int*);

    int readBuffer(int, const char *, int, int*);

}

#endif //ADDER_IOPOSIX_H
