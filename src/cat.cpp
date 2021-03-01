#include <iostream>
#include <fcntl.h>
#include <cctype>
#include <sstream>
#include "options/options.h"
#include "ioposix/ioposix.h"

std::string convertToHexified(const char* buf, int size);

int main(int argc, char **argv) {

    Options options(argc, argv);

    if (options.checkAndHandleOption({"-h", "--help"})) {
        std::cout << "Usage: mycat [OPTION]... [FILE]...\nConcatenate FILE(s) to standard output." << std::endl;
        return EXIT_SUCCESS;
    }

    bool isHexified(false);
    if (options.checkAndHandleOption({"-A"})) {
        isHexified = true;
    }

    auto fileNames = options.getOtherOptions();

    std::vector<int> fd_v;
    int openedFd;

    for (const auto& fileName: fileNames) {

        if ((openedFd = open(fileName.c_str(), O_RDONLY)) < 0) {
            std::cerr << "Error while opening file " << fileName << std::endl;
            return EXIT_FAILURE;
        }

        fd_v.push_back(openedFd);
    }

    int BUF_MAX_SIZE = 512;
    char buf[512];

    for (const auto& fd: fd_v) {

        int readSize = -1;
        int *readStatus = nullptr;
        int writeSize;
        int *writeStatus = nullptr;

        while (readSize != 0) {
            if ((readSize = ioposix::readBuffer(fd, buf, BUF_MAX_SIZE, readStatus)) == -1) {
                std::cerr << "Error while reading file" << std::endl;
                return EXIT_FAILURE;
            }

            const char* hexBuf = buf;
            if (isHexified) {
                auto hexStr = convertToHexified(buf, readSize);
                readSize = hexStr.size();
                hexBuf = hexStr.c_str();
            }

            if ((writeSize = ioposix::writeBuffer(ioposix::FD_STDOUT, hexBuf, readSize, writeStatus)) == -1) {
                std::cerr << "Error while writing to stdout" << std::endl;
                return EXIT_FAILURE;
            }
        }

        close(fd);
    }


    return EXIT_SUCCESS;
}

std::string convertToHexified(const char* buf, int size) {

    std::string hexStr;
    std::ostringstream os;
    std::string hexSymbol;

    for (int i = 0; i < size; i++) {

        if (isprint(buf[i]) == 0 && isspace(buf[i]) == 0 ) {
            os << std::hex << std::uppercase << (int) buf[i];
            hexSymbol += "\\x";
            hexSymbol += os.str().substr(os.str().size() - 2, 2);
        }
        else hexSymbol = buf[i];

        hexStr += hexSymbol;
    }

    return hexStr;
}

