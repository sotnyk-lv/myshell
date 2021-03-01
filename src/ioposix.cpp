#include <random>
#include "ioposix/ioposix.h"

int ioposix::writeBuffer(int fd, const char *buf, int size, int *status) {
    int writtenBytes = 0;
    while (writtenBytes < size) {
        int writtenNow = write(
                fd, buf + writtenBytes, size - writtenBytes);

        if (writtenNow == -1) {
            if (errno == EINTR) continue;
            else {
                *status = errno;
                return -1;
            }
        }
        else {
            writtenBytes += writtenNow;
        }
    }
    return writtenBytes;
}

int ioposix::readBuffer(int fd, const char *buf, int size, int *status) {
    int readBytes = 0;
    int readNow = -1;
    while (readNow != 0 && readBytes < size) {

        readNow = read(fd, (void *) (buf + readBytes), size - readBytes);

        if (readNow == -1) {
            if (errno == EINTR) continue;
            else {
                *status = errno;
                return -1;
            }
        }
        else {
            readBytes += readNow;
        }
    }
    return readBytes;
}
