#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <exception>
#include "shell/shell.h"

namespace po = boost::program_options;
bool processCommandLine(int argc, char** argv,
                        unsigned short& port)
{
    int port_number;
    try
    {
        po::options_description visible("Program Usage", 1024, 512);
        visible.add_options()
                ("help,h",     "produce help message");

        po::options_description hidden("Hidden options");
        hidden.add_options()
                ("port,p", po::value<int>(&port_number), "set the server port");

        po::options_description all("All options");
        all.add(visible).add(hidden);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(all).run(), vm);
        po::notify(vm);

        if (vm.count("help"))
        {
            std::cout << visible << "\n";
            return false;
        }

        if (vm.count("port"))
        {
            printf("po: port: %d\n", port_number);
            port = port_number;
            return true;
        }
        po::notify(vm);
    }
    catch(std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return false;
    }
    catch(...)
    {
        std::cerr << "Unknown error!" << "\n";
        return false;
    }

    return true;
}

int guard(int n, const char * err) { if (n == -1) { perror(err); exit(1); } return n; }

int main(int ac, char** av) {
    std::cout << "[+] Starting server:\t" <<  "\n";
    unsigned short port;

    bool result = processCommandLine(ac, av, port);
    if (!result) {
        std::cout << "> port:\t" << port << "\n";
        return -1;
    }
    int listen_fd = guard(socket(PF_INET, SOCK_STREAM, 0), "Could not create TCP socket");
    printf("[+]Created new socket %d\n", listen_fd);

    struct sockaddr_in listen_addr{};

    if (port) {
        int opt = 1;
        guard(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)),
              "Couldn't set options");


        listen_addr.sin_family = AF_INET;
        listen_addr.sin_addr.s_addr = INADDR_ANY;
        listen_addr.sin_port = htons( port );

        guard(bind(listen_fd, (struct sockaddr *)&listen_addr, sizeof(listen_addr)), "Could not bind");

    }
    guard(listen(listen_fd, 100), "Could not listen on TCP socket");

    socklen_t addr_len = sizeof(listen_addr);
    guard(getsockname(listen_fd, (struct sockaddr *) &listen_addr, &addr_len), "Could not get socket name");
    printf("> Listening for connections on port %d\n", ntohs(listen_addr.sin_port));


    for (;;) {
        int conn_fd = accept(listen_fd, nullptr, nullptr);
        printf("> [+] Got new connection %d\n", conn_fd);

        if (guard(fork(), "Could not fork") == 0) {
            pid_t my_pid = getpid();
            printf("> [+] %d: forked\n", my_pid);

            char buffer[1024];
            bzero(buffer, sizeof(buffer));
            shell curr_shell;
            shell::getPwd(buffer);
            send(conn_fd, buffer, strlen(buffer), 0);
            bzero(buffer, sizeof(buffer));

            for (;;) {
                ssize_t num_bytes_received = guard(recv(conn_fd, buffer, sizeof(buffer), 0),
                                                   "Could not recv on TCP connection");
                if (strncmp(buffer, ":q", 2) == 0) {
                    printf("[-] Disconnected from %s:%d\n", inet_ntoa(listen_addr.sin_addr),
                           ntohs(listen_addr.sin_port));
                    break;
                } else {
                    if (buffer[strlen(buffer) - 1] == '\n' && buffer[strlen(buffer) - 2] == '\r') {
                        buffer[strlen(buffer) - 1] = 0;
                        buffer[strlen(buffer) - 1] = 0;
                    }
                }

                if (num_bytes_received == 0) {
                    printf("%d: received end-of-connection; closing connection and exiting\n", my_pid);
                    guard(shutdown(conn_fd, SHUT_WR), "Could not shutdown TCP connection");
                    guard(close(conn_fd), "Could not close TCP connection");
                    exit(0);
                }

                printf("  > [*] Client: %s; buffer size: %lu\n", buffer, strlen(buffer));
                curr_shell.execLine(buffer);
                guard(send(conn_fd, buffer, strlen(buffer), 0), "Could not send to TCP connection");
                bzero(buffer, sizeof(buffer));
            }
        } else {
            // Child takes over connection; close it in parent
            close(conn_fd);
        }
    }
}

