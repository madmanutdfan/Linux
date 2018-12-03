#include <sys/stat.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include "CopyFileDaemon.h"

int main() {
	// "��� ���������", PID ��������� � ������ ��������� | ����� �������� � ��������� �������, ��������� ����������������� ������
	openlog("copy daemon", LOG_PID | LOG_CONS, LOG_USER);

	//������ ���� �������� �������� ��������
	char dir[PATH_MAX];
	getcwd(dir, PATH_MAX); 

	// ���� � ����� ������������
	CopyFileDaemon::s_configFile = strcat(dir, "/copyfiledaemon.conf"); 

	pid_t pid = fork();
	if (pid < 0) {
		// ��������� ���������
		syslog(LOG_ERR, "fork failed");
		exit(EXIT_FAILURE);
	}
	if (pid > 0) {
		// �������������� ���������
		syslog(LOG_INFO, "forked succesfully");
		exit(EXIT_SUCCESS);
	}

	CopyFileDaemon::changePidFile();

	// c���� ����� ������ �������� ���������������� ������
	umask(0);

	// ������ ����� �����, ����� �� �������� �� ��������
	if (setsid() < 0) {
		syslog(LOG_ERR, "session creation failed");
		exit(EXIT_FAILURE);
	}
	syslog(LOG_INFO, "created session succesfully");

	// ��������� � ������ ����
	if (chdir("/") < 0) {
		syslog(LOG_ERR, "working directory change failed");
		exit(EXIT_FAILURE);
	}

	// ��������� �e��������� �����/������/������
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

