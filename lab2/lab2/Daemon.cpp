#include <sys/stat.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sys/ioctl.h>
#include <cstring>
#include <linux/limits.h>
#include <limits.h>
#include <direct.h>
#include <sys/types.h>
#include <dirent.h>
class CopyFileDaemon {
public:
	static void changePidFile();
	static void signalsHandler(int signum);
	static void runDaemon();
	static void executeCmd(int act, int type);
	static void readConfig();
	static std::string s_configFile;
	static std::string firstFolder;
	static std::string secondFolder;
	static int timer;

};

#define PID_FILE "/var/run/copyfiledaemon.pid"
enum { CreateFolder, CopyFile, OLD, NEW };
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
	}
	else {
		syslog(LOG_INFO, "create and write PID_FILE");
		pidFileWrite.open(PID_FILE);
	}
	if (!pidFileWrite.is_open()) {
		syslog(LOG_ERR, "can't write to PID_FILE");
		exit(EXIT_FAILURE);
	}
	pidFileWrite << getpid() << std::endl;
	pidFileRead.close();
	pidFileWrite.close();
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
void CopyFileDaemon::executeCmd(int act, int type) {
	std::string cmd;
	if (act == CreateFolder && type == OLD)
		sprintf(cmd.c_str, "mkdir %s/OLD", secondFolder);
	if (act == CreateFolder && type == NEW)
		sprintf(cmd.c_str, "mkdir %s/NEW", secondFolder);
	if (act == CopyFile && type == NEW)
		sprintf(cmd.c_str, "cp `find %s/* -mmin -3` %s/NEW/", firstFolder, secondFolder);
	if (act == CopyFile && type == OLD)
		sprintf(cmd.c_str, "cp `find %s/* -mmin +3` %s/OLD/", firstFolder, secondFolder);
	system(cmd.c_str);
	cmd.clear();
}
void CopyFileDaemon::runDaemon() {
	// создаем папку OLD
	executeCmd(CreateFolder, OLD);

	// создаем папку NEW
	executeCmd(CreateFolder, NEW);
	while (true) {
		// копируем файлы не старше 3 минут
		executeCmd(CopyFile, NEW);
		// копируем файлы старше 3 минут
		executeCmd(CopyFile, OLD);

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
		if (k == 2)
			timer = atoi(str.c_str);
		k++;
	}
	file.close();
}

int main() {
	// "имя программы", PID программы в каждое сообщение | вывод напрямую в системную консоль, сообщения пользовательского уровня
	openlog("copy daemon", LOG_PID | LOG_CONS, LOG_USER);

	//полный путь текущего рабочего каталога
	char dir[PATH_MAX];
	getcwd(dir, PATH_MAX);

	// путь к файлу конфигурации
	CopyFileDaemon::s_configFile = strcat(dir, "/copyfiledaemon.conf");

	pid_t pid = fork();
	if (pid < 0) {
		// ошибочное состояние
		syslog(LOG_ERR, "fork failed");
		exit(EXIT_FAILURE);
	}
	if (pid > 0) {
		// информационное сообщение
		syslog(LOG_INFO, "forked succesfully");
		exit(EXIT_SUCCESS);
	}

	CopyFileDaemon::changePidFile();

	// cброс маски режима создания пользовательских файлов
	umask(0);

	// создаём новый сеанс, чтобы не зависеть от родителя
	if (setsid() < 0) {
		syslog(LOG_ERR, "session creation failed");
		exit(EXIT_FAILURE);
	}
	syslog(LOG_INFO, "created session succesfully");

	// переходим в корень диск
	if (chdir("/") < 0) {
		syslog(LOG_ERR, "working directory change failed");
		exit(EXIT_FAILURE);
	}

	// закрываем дeскрипторы ввода/вывода/ошибок
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = &CopyFileDaemon::signalsHandler;
	sigset_t set;


	sigemptyset(&set);
	sigaddset(&set, SIGHUP);
	sigaddset(&set, SIGTERM);
	act.sa_mask = set;
	sigaction(SIGHUP, &act, 0);
	sigaction(SIGTERM, &act, 0);
	syslog(LOG_INFO, "daemon is running");

	CopyFileDaemon::readConfig();
	CopyFileDaemon::runDaemon();
	return EXIT_SUCCESS;
}
