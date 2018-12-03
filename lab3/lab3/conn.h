#include"goatlings.h"
#include <fcntl.h> 
#include <sys/mman.h> 
class Conn {
public:
	//
	// host or client
	//
	bool            hoc;

	//
	// pipe
	//
	int             fd[2][2];
	int				nWR;

	//
	// mmap
	//
	void*           addr_mmap;
	
	//
	// shm
	//
	int             shm;
	void*           addr_shm;

	//
	// func
	//
	                Conn    (int id, bool create);
	static void     Create  (int id)
	void            Read    (void *buf, size_t count);
	void            Write   (void *buf, size_t count);
	                ~Conn   ();
};

Conn*               from_host[nGoatlings];
Conn*               from_client[nGoatlings];