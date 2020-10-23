#include "server.h"

/**
 * binds a host:port socket, then calls this->Listen to listen for incoming connections
 */
Server::Server(const std::string& host, const unsigned short port)
    : host(std::move(host)), port(port)
{
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->sockfd < 0) {
        this->error("ERROR opening socket");
    }
    
    sockaddr_in hint; 
    hint.sin_family = AF_INET;
    hint.sin_port = htons(this->port);
    inet_pton(AF_INET, host.c_str(), &hint.sin_addr);

    if (bind(this->sockfd, (sockaddr*)&hint, sizeof(hint)) < 0) {
        this->error("ERROR can't bind to ip/port");
    }

    this->Listen();
}

// closes the server socket
Server::~Server()
{
    close(this->sockfd);
}

/**
 * listen for incoming connections and spawn threads for each connection
 */
void Server::Listen()
{
    if (listen(this->sockfd, SOMAXCONN) < 0) {
        this->error("ERROR can't listen");
    }

    while (true) {
        socklen_t clientSize = sizeof(this->clientAddress);

        // accept connections
        int clientSocket = accept(this->sockfd, (sockaddr*)&this->clientAddress, &clientSize);

        if (fork() != 0) {
            close(clientSocket);
            continue;
        }

        if (clientSocket < 0) {
            std::cerr << "ERROR problem with client connection" << std::endl;
        } else {
            this->handleConnection(clientSocket);
        }
    }
}

/**
 * handles a single client connection
 */
void Server::handleConnection(int client) {
    // allocate stack buffers for host and service strings
    char host[NI_MAXHOST];
    char svc[NI_MAXSERV];

    // set all chars to null in each buffer
    memset(host, 0, NI_MAXHOST);
    memset(svc, 0, NI_MAXSERV);
    
    // resolve a hostname if possible
    int result = getnameinfo((sockaddr*)&this->clientAddress, sizeof(this->clientAddress), host, NI_MAXHOST, svc, NI_MAXSERV, 0);

    // if hostname resolved, print it, else print the IP
    if (result) {
        std::cout << host << " connected on " << svc << std::endl;
    } else {
        inet_ntop(AF_INET, &this->clientAddress.sin_addr, host, NI_MAXHOST);
        std::cout << host << " connected on " << ntohs(this->clientAddress.sin_port) << std::endl;
    }

    // allocate a stack buffer for recieved data
    char buf[4096];

    // read and handle bytes until the connection ends
    while (true) {
        // set the recv buffer to null chars
        memset(buf, 0, 4096);

        // read data incoming from the client into the recv buffer
        int bytesRecv = recv(client, buf, 4096, 0);

        switch (bytesRecv) {
            case -1: // connection error
                std::cerr << "There was a connection issue with " << host << std::endl;
                goto srv_disconnect; // disconnect
                break;
            case 0: // client disconnected
                std::cout << host << " disconnected" << std::endl;
                goto srv_disconnect; // disconnect
                break;
            default:
                std::cout << "Recieved: " << std::string(buf, 0, bytesRecv) << std::endl;
                // DO HTTP STUFF HERE
                // echo the contents of the recv buffer back to the client
                send(client, buf, bytesRecv + 1, 0);
                break;
        }
    }

srv_disconnect:
    close(client);
}

// for fatal errors that should kill the program.
void Server::error(const std::string& message)
{
    std::cerr << message << std::endl;
    std::exit(1);
}

