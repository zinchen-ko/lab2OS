#include <sys/ioctl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <malloc.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define WR_VALUE _IOW('a','a',struct message*)


struct task_struct_info {
	int prio;
	int static_prio;
	int normal_prio;
	unsigned int rt_priority;

};

struct socket_info {
	short type;
	unsigned long flags;
};

struct message {
	struct task_struct_info* tsi;
	struct socket_info* si;
	pid_t pid;
	int fd;
	int *err;
};


int main(int argc, char *argv[]) {
	struct message msg;
	struct socket_info si;
	struct task_struct_info tsi;
	int err;
	int fde;

 	msg.pid = atoi(argv[1]);
	msg.tsi = &tsi;
	msg.si = &si;
	msg.err = &err;


	printf("\nOpening Driver\n");
	fde = open("/dev/etx_device", O_RDWR);

	if(fde < 0) {
		printf("Cannot open device file...\n");
		return 0;
	}
	int s = socket(AF_INET, SOCK_STREAM, 0);
	msg.fd = s;

	printf("Writing data to Driver\n");
	ioctl(fde, WR_VALUE, (struct message*) &msg);
	
	
	printf ("Prio: %d\n", msg.tsi->prio);
	printf ("Normal Prio: %d\n", msg.tsi->normal_prio);

	printf ("Type: %d\n", msg.si->type);
	printf ("Flags: %lu\n", msg.si->flags);

	printf("\nClosing Driver\n");
	close(fde);
}
