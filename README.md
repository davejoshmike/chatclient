To Compile on Linux (with g++):

1. cd ~/chatclient/
2. g++ -o talkclient talkclient.cpp
3. ./talkclient

usage: ./talkclient [-v, --verbose] [-s, --server SERVER]
		    [-p, --port PORT] [-n, --name NAME] [-h]

optional arguments:
	-v, --verbose  turn verbose output on
	-s, --server SERVER  IP address or name of server that the client is connecting to (default 127.0.0.1)
	-p, --port PORT  TCP port the client is connecting to (default 12345)
	-n, --name NAME  the name you wish to be referred to (default hostname)
	-h, --help	 display this help and exit
