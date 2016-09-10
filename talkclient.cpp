/*
 * talkclient connects to a talk server and allows you to send and 
 * 	receive messages from other talkclient instances
 *
 *  Created on: Mar 7, 2016
 *  Completed on: April 7, 2016
 *      Author: djm43
 *
 * Relied heavily on:
 *  http://beej.us/guide/bgnet/output/html/multipage/syscalls.html
 *  http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#gethostname
 *  using ch 5 for getaddrinfo(), socket(), connect(), accept()
 *  	send(), close() and freeaddrinfo()
 *  using ch 7.2 for select()
 *  using ch 5.11 for gethostname()
 */

#define STDIN 0 //file descriptor
#include "talkclient.h"

enum Argument { "-v", "--verbose" = 0, "-s", "--server" = 1, "-p", "--port" = 2, "-n", "--name" = 3, "-h", "--help" = 4 }

int main (int argc, char** argv) {
	talkclient *tc = new talkclient(argc, argv);

	struct sockaddr_storage their_addr; //accept() part

	fd_set master; //set of all connected sockets
	fd_set readfds; //temp set of sockets ready to be read
	fd_set writefds; //temp set of sockets ready to be written

	//File Descriptors
	int fdmax; //maximum file descriptor
	int listener; //listening socket descriptor
	int new_fd; //accept()'s new socket descriptor
	int sockfd; //socket file descriptor

	char buf[1024];
	int len = 1024; //length of buf (read up to 1024 bytes)
	int bytes_received;

	int optval = 1; //for use in setsockopt()

	//addrinfos
	struct addrinfo hints;
	struct addrinfo *ai;
	struct addrinfo *res; //points to a list of addrinfos which each contain a sockaddr

	socklen_t addrlen;
	socklen_t addr_size; //accept() part


	//Timeout values for use in select()
	struct timeval tv; //timeout value
	tv.tv_sec = 2;
	tv.tv_usec = 500000;

	FD_ZERO(&master); //clear master set
	FD_ZERO(&readfds); //clear temp set
	FD_SET(STDIN, &master);
	readfds = master;

	memset(&hints, 0, sizeof hints); //make sure struct is empty
	hints.ai_family = AF_UNSPEC; //Don't care if IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; //TCP socket
	hints.ai_flags = AI_PASSIVE; //fill in my IP for me
	sockfd = getaddrinfo(tc->getServer().c_str(), tc->getPort().c_str(), &hints, &res);
	if(sockfd !=0){
		cerr << gai_strerror(sockfd) << flush;
		exit(1);
	}
	if(tc->isVerbose()){
		cout << "getaddrinfo()ed " << flush;
	}

	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sockfd == -1){
		cerr << gai_strerror(sockfd) << flush;
		exit(1);
	}

	if(tc->isVerbose()){
		cout << "socked()ed " << flush;
	}

	//connect to the server!
	if(connect(sockfd, res->ai_addr, res->ai_addrlen) == -1){
		cerr << "Could not connect!" << endl;
		close(sockfd);
		exit(1);
	}
	//FD_SET(sockfd, &master);
	cout << "connected " << "on socket " << sockfd << "!" << endl;

	FD_SET(sockfd, &master);
	fdmax = sockfd;

	while(true){
		readfds = master; //copy master set into ready-to-read set
		writefds = master; //copy master set into ready-to-write set

		for(int i=0;i<= fdmax; i++){
			//		//if(new_fd ==0){
			//exisiting server

			if(select(fdmax+1, &readfds, NULL, NULL, &tv) == -1){
				cerr << "select() error" << endl;
				exit(1);
			}
			if(FD_ISSET(sockfd, &readfds)){
				//receive messages from server				
				if(tc->isVerbose()){
					//cout << "Receiving... " << flush;
				}
				
				addr_size = sizeof their_addr;
				new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

				for(int erase = 0;erase<1024;erase++){
					buf[erase] = '\0'; //clear out the previous message
				}
				bytes_received = recv(sockfd, buf, len, 0);
				if(bytes_received == 0){
					cerr << " connection closed" << endl;
					return -1;
				}
				string msg = (string)buf;
				cout << msg << endl;
			}

			if(select(fdmax+1, &writefds, NULL, NULL, &tv) == -1){
						cerr << "select() error" << endl;
						exit(1);
					}
			if(FD_ISSET(STDIN, &writefds)){
				//send messages from host				
				if(tc->isVerbose()){
					//cout << "Sending... " << flush;
				}				
				string input;
				getline(cin, input);
				string myMsg = tc->getUserName().c_str();
				myMsg += " says: ";
				myMsg += (string)input;
				char *msg = (strdup)(myMsg.c_str());
				int len, bytes_sent;
				len = strlen(msg);
				bytes_sent = send(sockfd, msg, len , 0);
				if(bytes_sent <= 0){
					cout << "Connection Closed! Exiting..." << endl;
					break;
				}
			}
		}

	}
	close(sockfd);
	freeaddrinfo(res);
	exit(1);
}


talkclient::talkclient(int argc, char** argv){
	//default
	verbose = false;
	server = "127.0.0.1";
	port = "12345";

	//http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#gethostname
	char name[32];
	int errorCheck = gethostname(name, sizeof name);
	if(errorCheck == -1) {
		cerr << gai_strerror(errorCheck) << flush;
		exit(1);
	}
	userName = string(name);

	//check command line
	for(int i=1;i<argc;i++){
		Argument argument = string(argv[i]);
		switch(argument){
		//verbose
		case 0:
			verbose = true;
			i--;
			break;
		//IP address or server name
		case 1:
			server = string(argv[i+1]);
			break;
		//TCP port the server is listening on
		case 2:
			//port = atoi(argv[i+1]); //wrap in try catch
			port = string(argv[i+1]);
			break;
		//The name by which you are identified by
		case 3:
			userName = argv[i+1]; //wrap in try catch
			break;
		default:
                        cerr << "Bad Argument: " << string(argv[i]) << endl;
		//help
		case 4:
			cout << "usage: ./talkclient [-v, --verbose] [-s, --server SERVER]\n\t\t    [-p, --port PORT] [-n, --name NAME] [-h]" << endl;
			cout << "\noptional arguments:" << endl;
			cout << "\t-v, --verbose  turn verbose output on" << endl;
			cout << "\t-s, --server SERVER  IP address or name of server that the client is connecting to (default 127.0.0.1)" << endl;
			cout << "\t-p, --port PORT  TCP port the client is connecting to (default 12345)" << endl;
			cout << "\t-n, --name NAME  the name you wish to be referred to (default hostname)" << endl;
			cout << "\t-h, --help\t display this help and exit" << endl;
			exit(1);
		//default:
		//	cerr << "Bad Argument: " << string(argv[i]) << endl;
		//	exit(1);
		}
		i++;
	}
	if(verbose) {
		cout << "Server: " << server << flush;
		cout << " | ";
		cout << "Port: " << port << flush;
		cout << " | ";
		cout << "Name: " << userName << endl;
	}
}

string talkclient::getUserName() const {
	return userName;
}

string talkclient::getServer() const {
	return server;
}

string talkclient::getPort() const {
	return port;
}

bool talkclient::isVerbose() const {
	return verbose;
}

//enum Argument {
//	"-v", "--verbose" = 0,
//	"-s", "--server" = 1,
//	"-p", "--port" = 2,
//	"-n", "--name" = 3,
//	"-h", "--help" = 4
//};
