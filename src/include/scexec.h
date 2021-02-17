#ifndef SCEXEC_H
#define SCEXEC_H

#include <stdio.h>

#define SCEXEC_EARGV -1
#define SCEXEC_EFORK -2
#define SCEXEC_ESUID -3
#define SCEXEC_ESGID -4

#define SCEXEC_MAX_ARGS 1024
#define SCEXEC_MAX_DATA 32768

#ifndef SCEXEC_NAME
	#define SCEXEC_NAME "scexec"
#endif

#define scexec_err(s)            fputs("[err] " s "\n", stderr)
#define scexec_fmt_err(fmt, ...) fprintf(stderr, "[err] " fmt "\n", __VA_ARGS__)
#endif
