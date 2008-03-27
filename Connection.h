#ifndef CONNECTION_H
#define CONNECTION_H

#include <fcntl.h>
#include <iostream>
#include <pty.h>
#include "Globals.h"

/* local nethack */
#define LOCAL_NETHACK "/usr/games/nethack"
#define LOCAL_USLEEP 200000
/* remote nethack */
#define REMOTE_NETHACK ""
#define REMOTE_USLEEP 2000000

using namespace std;

class Connection {
	public:
		/* variables */
		bool remote;

		/* constructors */
		Connection(bool remote);

		/* destructors */
		~Connection();

		/* methods */
		ssize_t retrieve(char *buffer, size_t count);
		ssize_t send(const char *buffer);

	private:
		/* variables */
		int input[2];
		int output[2];
		unsigned long usleep_time;
};

#endif