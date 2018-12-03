#include "CopyFileDaemon.h"
std::string CopyFileDaemon::s_configFile;

void CopyFileDaemon::changePidFile() {
	std::ifstream pidFileRead; 
	std::ofstream pidFileWrite;

	// защита от повторного запуска
	pidFileRead.open(PID_FILE);
	syslog(LOG_INFO, "change PID_FILE");
	if (pidFileRead.is_open()) {
		int oldPid;
		pidFileRead >> oldPid;
		syslog(LOG_INFO, "kill old daemon process %d", oldPid);
		kill(oldPid, SIGTERM);
		pidFileWrite.open(PID_FILE, std::ios::out | std::ios::trunc);
		if (!pidFileWrite.is_open()) {
			syslog(LOG_ERR, "can't write to PID_FILE");
			exit(EXIT_FAILURE);
		}
		pidFileWrite << getpid() << std::endl;
		pidFileRead.close();
	}
	else {
		syslog(LOG_ERR, "create and write PID_FILE");
		pidFileWrite.open(PID_FILE);
		if (!pidFileWrite.is_open()) {
			syslog(LOG_ERR, "can't write to PID_FILE");
			exit(EXIT_FAILURE);
		}
		pidFileWrite << getpid() << std::endl;
		pidFileWrite.close();
	}
	
}

void CopyFileDaemon::signalsHandler(int signum) {
	switch (signum) {
	case SIGHUP:
		syslog(LOG_INFO, "signal SIGHUP");
		CopyFileDaemon::readConfig();
		break;
	case SIGTERM:
		syslog(LOG_INFO, "signal SIGTERM");
		unlink(PID_FILE);
		syslog(LOG_INFO, "exit, shut down");
		closelog();
		exit(EXIT_SUCCESS);
		break;
	}
}

void CopyFileDaemon::runDaemon() {
	std::string cmd;
	// создаем папку OLD
	sprintf(cmd.c_str, "mkdir %s/OLD", secondFolder);
	system(cmd.c_str);
	cmd.clear();

	// создаем папку NEW
	sprintf(cmd.c_str, "mkdir %s/NEW", secondFolder);
	system(cmd.c_str);
	cmd.clear();

	while (true) {
		std::string copyFileCmd;
		// копируем файлы не старше 3 минут
		sprintf(copyFileCmd.c_str, "cp `find %s/* -mmin -3` %s/NEW", firstFolder, secondFolder);
		system(copyFileCmd.c_str);
		copyFileCmd.clear();

		// копируем файлы старше 3 минут
		sprintf(copyFileCmd.c_str, "cp `find %s/* -mmin +3` %s/OLD", firstFolder, secondFolder);
		system(copyFileCmd.c_str);
		copyFileCmd.clear();

		//ждем 1 минутy до следующей итерации
		sleep(timer);
	}
}

void CopyFileDaemon::readConfig() {
	syslog(LOG_INFO, "reread copyfiledaemon.config");

	std::ifstream file;
	syslog(LOG_INFO, "read %s", s_configFile.c_str());
	file.open(s_configFile.c_str());

	std::string str;
	int k = 0;
	while (std::getline(file, str) && k < 3) {
		if (strlen(str.c_str)) {
			syslog(LOG_ERR, "empty string");
			break;
		}
		if (k == 0)
			std::strcpy(firstFolder.c_str, str.c_str);
		if (k == 1)
			std::strcpy(secondFolder.c_str, str.c_str);
		if (k = 2)
			timer = atoi(str.c_str);
		k++;
	}
	file.close();
}

