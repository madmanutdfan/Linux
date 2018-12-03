#include <sys/stat.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include "CopyFileDaemon.h"

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

