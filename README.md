# CPP-Chat-Server
Final project for the course Programming in C++ (T-403-FORC) at Reykjavík University.

TCP server-client chat on windows- there is no implementation for linux systems

## To Compile and Run

pthread and winsock2 needed for compilation

in files /server/myServer.h and /client/myClient.h need to add the local absolute path to file \Net\includeMe.h in the include.

To compile the server run: g++ \Net\*.cpp \Server\*.cpp -o server -pthread -lws2_32

To compile the client run: g++ \Net\*.cpp \Client\*.cpp -o client -pthread -lws2_32

To run the server: ./server.exe {ip_string} {port_number}
To run the client: ./client.exe {ip_string} {port_number}

ip_string can be ipv4 or ipv6