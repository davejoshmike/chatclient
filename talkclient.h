/*
 * talkclient.h
 *
 *  Created on: Mar 7, 2016
 *      Author: djm43
 */

#ifndef TALKCLIENT_H_
#define TALKCLIENT_H_

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <limits.h> //http://stackoverflow.com/questions/27914311/get-computer-name-and-logged-user-name

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

using namespace std;

class talkclient {
public:
	talkclient(int argc, char** argv);
	string getServer() const;
	string getPort() const;
	string getUserName() const;
	bool isVerbose() const;
private:
	bool verbose;
	string server;
	string port;
	string userName;
};

#endif /* TALKCLIENT_H_ */
