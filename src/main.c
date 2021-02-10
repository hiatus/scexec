#include "scexec.h"
#include "shannon.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

struct scexec_opts {
	char verbose;
	char quit;
	char fork;
	char suid;
	char sgid;

	uid_t uid;
	gid_t gid;
};

static const char banner[] =
TARGET " [options] [file]?\n"
"	-h        help\n"
"	-v        show data information\n"
"	-q        show data information and quit\n"
"	-f        fork prior to data execution\n"
"	-u [id]   set the effective user id to [id]\n"
"	-g [id]   set the effective group id to [id]\n";

static ssize_t read_data(const char *fn, unsigned char *buf)
{
	FILE *fp;
	ssize_t ret;

	if (fn) {
		if (! (fp = fopen(fn, "rb"))) {
			perror("fopen");
			return -1;
		}

		if ((ret = fread(buf, 1, SCEXEC_MAX_DATA, fp)) < 0)
			perror("fread");

		fclose(fp);
		return ret;
	}

	if ((ret = read(STDIN_FILENO, buf, SCEXEC_MAX_DATA)) < 0)
		perror("read");

	return ret;
}

int main(int argc, char **argv)
{
	int opt;
	pid_t pid;
	ssize_t len;

	unsigned char data[SCEXEC_MAX_DATA];

	struct scexec_opts opts = {
		.verbose = 0,
		.quit    = 0,
		.fork    = 0,
		.uid     = getuid(),
		.gid     = getgid()
	};

	while ((opt = getopt(argc, argv, ":hvqfu:g:")) != -1) {
		switch (opt) {
			case 'h':
				fputs(banner, stderr);
				return 0;

			case 'v':
				opts.verbose = 1;
				break;

			case 'q':
				opts.quit = opts.verbose = 1;
				break;

			case 'f':
				opts.fork = 1;
				break;

			case 'u':
				opts.uid = (uid_t)strtoul(optarg, NULL, 10);
				break;

			case 'g':
				opts.gid = (gid_t)strtoul(optarg, NULL, 10);
				break;

			case ':':
				scexec_fmt_err("Missing argument for '%c'", optopt);
				return -SCEXEC_EARGV;

			case '?':
				scexec_fmt_err("Invalid option: '%c'", optopt);
				return -SCEXEC_EARGV;
		}
	}

	len = (optind < argc) ?
	      read_data(argv[optind], data) : read_data(NULL, data);

	if (! len)
		scexec_err("Input size is 0");

	if (len <= 0)
		return -SCEXEC_EARGV;

	shannon_init();
	shannon_update(data, len);

	if (opts.verbose) {
		printf("Size    : %zu\n", len);
		printf("Entropy : %.5f\n\n", shannon_entropy());

		for (ssize_t i = 1; i <= len; ++i) {
			printf("%02x", data[i - 1]);
			putchar((i % 16) ? ' ' : '\n');
		}

		if (len % 16)
			putchar('\n');
	}

	if (opts.quit)
		return 0;

	if (opts.fork) {
		if ((pid = fork()) < 0) {
			perror("fork");
			return -SCEXEC_EFORK;
		}
		else
		if (pid)
			exit(0);

		if ((pid = setsid()) < 0) {
			perror("setsid");
			return -SCEXEC_EFORK;
		}
	}

	if (opts.uid != getuid() && setuid(opts.uid) < 0) {
		perror("setuid");
		return SCEXEC_ESUID;
	}

	if (opts.gid != getgid() && setgid(opts.gid) < 0) {
		perror("setgid");
		return SCEXEC_ESGID;
	}

	(*(void (*)(void))data)();

	return 0;
}