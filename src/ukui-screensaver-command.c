#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#define LENGTH 16

int main(int argc, char **argv)
{
	int pid;
	char pid_str[LENGTH] = {0};
	FILE *cmd;
	FILE *fp;
	char *file = "/tmp/ukui-screensaver-lock";

	if (argc == 2 && !strcmp(argv[1], "-l")) {
	} else {
		printf("Usage:\n");
		printf("  -l\tLock the screen\n");
		return 1;
	}

	cmd = popen("pidof -s ukui-screensaver", "r");
	fgets(pid_str, LENGTH, cmd);
	pclose(cmd);

	if (pid_str[0] == 0) {
		printf("ukui-screensaver process does not exist\n");
		return 1;
	}

	sscanf(pid_str, "%d", &pid);
	fp = fopen(file, "w");
	if (!fp) {
		printf("Can't open file %s\n", file);
		perror("Details: ");
		return 1;
	}
	fprintf(fp, "%d", pid);
	fclose(fp);
	kill(pid, SIGUSR1);
}

