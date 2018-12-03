#ifndef COPYFILEDAEMON_H
#define COPYFILEDAEMON_H

#include <stdio.h>
#include <signal.h>
#include <syslog.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <sys/ioctl.h>
#include <cstring>
#include <linux/limits.h>
#include <limits.h>

#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
class CopyFileDaemon {
public:
	static void changePidFile();
	static void signalsHandler(int signum);
	static void runDaemon();
	static void readConfig();
	static std::string s_configFile;
	static std::string firstFolder;
	static std::string secondFolder;
	static int timer;
};

#define PID_FILE "/var/run/copyfiledaemon.pid"

#endif

