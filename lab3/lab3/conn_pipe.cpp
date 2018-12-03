#include"conn.h"

Conn::Conn(int id, bool create) {
	nWR = 0;
	hoc = create;

	if (pipe(fd[0]) < 0 || pipe(fd[1]) < 0) {
		syslog(LOG_ERR, "ERROR::CREATE_PIPE\n");
		exit(1);
	}
}
void Conn::Create(int id) {
	from_client[id] = new Conn(0, false);
	goatlings[id] = new Goatling();
	syslog(LOG_INFO, "CLIENT::connection\n");
}
void Conn::Read(int* buf, size_t count) {
	if (hoc == false)  nWR = 0;
	else               nWR = 1;

	if (!read(fd[nWR][0], buf, (int)count)) {
		syslog(LOG_ERR, "ERROR::READ\n");
		exit(1);
	}
	else
		close(fd[nWR][1]);
}

void Conn::Write(int* buf, size_t count) {
	if (hoc == false)  nWR = 0;
	else               nWR = 1;

	if (!write(fd[nWR][1], buf, (int)count)) {
		syslog(LOG_ERR, "ERROR::WRITE\n");
		exit(1);
	}
	else
		close(fd[nWR][1]);
}
Conn::~Conn() {}