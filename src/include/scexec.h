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
	#define SCEXEC_NAME "scx"
#endif

#define scexec_err(s)            fputs("[err] " s, stderr)
#define scexec_log(s)            fputs("[" SCEXEC_NAME "] " s, stderr)

#define scexec_fmt_err(fmt, ...) fprintf(stderr, "[err] " fmt, __VA_ARGS__)
#define scexec_fmt_wrn(fmt, ...) fprintf(stderr, "[wrn] " fmt, __VA_ARGS__)
#define scexec_fmt_log(fmt, ...) fprintf(stderr, "[" SCEXEC_NAME "] " fmt, __VA_ARGS__)
#endif
