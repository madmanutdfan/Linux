#include"conn.h"
Conn::Conn(int id, bool create) {

	if(addr_mmap = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0) == MAP_FAILED)
		syslog(LOG_ERR, "ERROR::CREATE_ADDR\n");
		exit(1);
	}
}
void Conn::Create(int id) {
	from_client[id] = new Conn(0, false);
	goatlings[id] = new Goatling();
	syslog(LOG_INFO, "CLIENT::connection\n");
}
void Conn::Read(int* buf, size_t count) {
	if (!memcpy(buf, addr_mmap, sizeof(int))) {
		syslog(LOG_ERR, "ERROR::READ\n");
		exit(1);
	}
}
void Conn::Write(int* buf, size_t count) {
	if (!memcpy(addr_mmap, buf, sizeof(int))) {
		syslog(LOG_ERR, "ERROR::WRITE\n");
		exit(1);
	}
}
Conn::~Conn() {
	munmap(addr_mmap, sizeof(int));
}