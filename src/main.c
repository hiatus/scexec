#include "scexec.h"
#include "shannon.h"

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/prctl.h>

struct scexec_opts {
	uint8_t verbose;
	uint8_t quit;
	uint8_t fork;
	uint8_t setargv;

	uid_t uid;
	gid_t gid;

	size_t exec_count;
	size_t exec_inter;

	char args[SCEXEC_MAX_ARGS];
};

static const char banner[] =
TARGET " [options] [file]?\n"
"	-h          help\n"
"	-v          show data information\n"
"	-q          show data information and quit\n"
"	-f          fork prior to data execution\n"
"	-n [num]    execute data [num] times (0 == forever)\n"
"	-i [secs]   wait [secs] seconds between executions\n"
"	-a [argv]   set process name and arguments to [argv]\n"
"	-u [euid]   set the effective user id to [euid]\n"
"	-g [egid]   set the effective group id to [egid]\n";

// Set process arguments
static void _set_argv(int argc, char **argv, char *args)
{
	int argsc;
	char *token = strtok(args, " ");

	// Change process name to args[0]
	prctl(PR_SET_NAME, (token) ? token : "", 0, 0, 0);

	for (argsc = 0; token && argsc < argc; ++argsc) {
		strncpy(argv[argsc], token, strlen(argv[argsc]));
		token = strtok(NULL, " ");
	}

	for (; argsc < argc; ++argsc)
		memset(argv[argsc], 0x00, strlen(argv[argsc]));
}

static ssize_t _read_data(const char *fn, unsigned char *buf)
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

	uint8_t data[SCEXEC_MAX_DATA];

	struct scexec_opts opts = {
		.verbose    = 0,
		.quit       = 0,
		.fork       = 0,
		.setargv    = 0,
		.uid        = getuid(),
		.gid        = getgid(),
		.exec_count = 1,
		.exec_inter = 0
	};

	while ((opt = getopt(argc, argv, ":hvqfn:i:a:u:g:")) != -1) {
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

			case 'n':
				if (! (opts.exec_count = strtoul(optarg, NULL, 10))) {
					scexec_fmt_err(
						"Bad execution count: '%s'", optarg
					);

					return -SCEXEC_EARGV;
				}

				break;

			case 'i':
				opts.exec_inter = strtoul(optarg, NULL, 10);
				break;

			case 'a':
				opts.setargv = 1;
				strncpy(opts.args, optarg, SCEXEC_MAX_ARGS - 1);

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
	      _read_data(argv[optind], data) : _read_data(NULL, data);

	if (! len)
		scexec_err("Input size is 0");

	if (len <= 0)
		return -SCEXEC_EARGV;

	if (opts.setargv)
		_set_argv(argc, argv, opts.args);

	if (opts.verbose) {
		shannon_init();
		shannon_update(data, len);

		printf("Size    : %zu\n", len);
		printf("Entropy : %.5f\n\n", shannon_entropy());

		for (ssize_t i = 1; i <= len; ++i) {
			printf("%02x", data[i - 1]);
			putchar((i % 16) ? ' ' : '\n');
		}

		puts((len % 16) ? "\n" : "");
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

	for (size_t i = 1; i <= opts.exec_count; ++i) {
		if (i > 1)
			sleep(opts.exec_inter);

		if (opts.verbose)
			scexec_fmt_log("Execution iteration %zu", i);

		if ((pid = fork()) < 0) {
			perror("fork");

			scexec_fmt_wrn(
				"Sacrificing parent process on iteration %zu", i
			);

			(*(void (*)(void))data)();

			continue;
		}
		else
		if (! pid) {
			(*(void (*)(void))data)();
			_exit(0);
		}

		wait(NULL);
	}

	return 0;
}
