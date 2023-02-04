#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
	char received[1];

	int fds[2];
	pipe(fds);
	int cpid = fork();

    if(cpid < 0) {
        fprintf(2, "Error in pingpong.c\n");
        exit(1);
    }
    // child process
	if (cpid == 0) {
		
		read(fds[0], received, 1);

		int pid = getpid();
        printf("%d: received ping\n", pid);
		
		write(fds[1], received, 1);
	}
    else {
		write(fds[1], "1", 1);
		wait(0);

		read(fds[0], received, 1);

		int pid = getpid();
		printf("%d: received pong\n", pid);
	}

    exit(0);
}