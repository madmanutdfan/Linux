#include"conn.h"
#define SHARED_MEMORY_OBJECT_NAME "my_shared_memory"
#define SHARED_MEMORY_OBJECT_SIZE 11

Conn::Conn(int id, bool create) {
	if ((shm = shm_open(SHARED_MEMORY_OBJECT_NAME, mode | O_RDWR, 0777)) == -1) {
		syslog(LOG_ERR, "ERROR::CREATE_SHM\n");
		exit(1);
	}
	if (ftruncate(shm, SHARED_MEMORY_OBJECT_SIZE) == -1) {
		syslog(LOG_ERR, "ERROR::FTRUNCATE\n");
		exit(1);
	}
	if (addr_shm = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, shm, 0) == MAP_FAILED) {
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
	if (!memcpy(buf, addr_shm, sizeof(int))) {
		syslog(LOG_ERR, "ERROR::READ\n");
		exit(1);
	}
}

void Conn::Write(int* buf, size_t count) {
	if (!memcpy(addr_shm, buf, sizeof(int))) {
		syslog(LOG_ERR, "ERROR::WRITE\n");
		exit(1);
	}
}
Conn::~Conn() {
	munmap(addr_shm, SHARED_MEMORY_OBJECT_SIZE * sizeof(int));
	shm_unlink(SHARED_MEMORY_OBJECT_NAME);
}