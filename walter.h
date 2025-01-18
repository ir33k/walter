/* walter.h v4.2 from https://github.com/ir33k/walter by irek@gabr.pl

Walter is a single header library for writing unit tests in C made
with fewer complications by avoiding boilerplate.

Table of contents:

	Example
	Compile and run
	Disclaimers
	Changelog
	Licenses (at the very end of this file)

Example:

	// File: example.t.c
	#include "walter.h"

	TEST("Test message")            // Define test with assertions
	{
		// Basic assertions
		ASSERT(bool, "msg");    // Print message on false
		OK(bool);               // Is boolean true?
		EQ(b1, b2, size);       // Are buffers equal?
		NEQ(b1, b2, size);      // Are buffers not equal?
		SEQ(s1, s2);            // Are strings equal?
		SNEQ(s1, s2);           // Are strings not equal?
		return;                 // Force end of test

		// Run CMD with IN standard input expecting OUT
		// standard output and ERR standard error and CODE
		// exit code.  When file path for IN, OUT, ERR are
		// NULL then this part of input/output is ignored.
		//
		//   CMD          IN        OUT        ERR       CODE
		RUN("grep wh_",  "in.txt", "out.txt", "err.txt", 0);
		RUN("sed -i",    "in.txt",  NULL,      NULL,     1);
		RUN("./demo0.t",  NULL,    "out.txt", "err.txt", 5);
		RUN("ls -lh",     NULL,    "out.txt",  NULL,     0);
		RUN("pwd",        0,        0,         0,        0);

		// If you want to use string literal instead of file
		// path then use S2F() macro as argument that creates
		// temporary file with provided string.
		RUN("tr abc 123", S2F("AaBbCc"), S2F("A1B2C3"), 0, 0);
	}
	TEST("Another test 1") { ... }  // Define as many as WH_MAX
	SKIP("Another test 2") { ... }  // Skip or just ignore test
	SKIP("Another test 3") {}       // Body can be empty
	SKIP("TODO test 4")    {}       // Can be used for TODOs
	ONLY("Another test 5") { ... }  // Ignore all other tests

	// There is no "main()" function.

Compile and run:

	$ cc -o example.t example.t.c   # Compile
	$ ./example.t -h                # Print help
	$ ./example.t                   # Run tests
	$ echo $?                       # Number of failed tests

Disclaimers:

	Library can be included only in one test program because it
	has single global tests state, it defines it's own "main"
	function and TEST macro relays on file line numbers.

	It's expected that variables, functions and macros that are
	not mentioned in example are not used in test programs.

	You can define only 256 tests per file.  If you need more then
	simply change the value of WH_MAX in your copy of walter.h.

	When buffer or string assertion fails it prints part of that
	buffer or string as preview.  If you need different length of
	that preview then modify WH_SHOW macro value.

	In general I would like to encourage you to modify source code
	according to your needs.  If some macro name is in conflict to
	your existing macro then rename it.  If you need custom assert
	macro, then add it.  Source code is short and easy to extend.

	WH_ prefix stands for Walter.H.  _WH_ is used for private
	stuff.  __WH_ is for super epic internal private stuff, just
	move along, this is not the code you are looking for \(-_- )

Changelog:

	2024.01.09	v4.1

	1. Contribution by Kian-Meng Ang <kianmeng@cpan.org>: Fix
	   typos in comments.
	2. Add -a flag for printing absolute file paths.
	3. Remove arrow when printing error message for strings and
	   buffers assertions.  It was often pointing at wrong byte.
	   This was because string or buffer could contain characters
	   like tabs, returns, multi-byte or non printable characters.

	2023.12.04	v4.0

	1. Major version update because of breaking changes in SRUN().
	2. Remove SRUN() in favor of RUN() used with S2F().
	3. Implement S2F() macro and adjust documentation accordingly.
	4. Fix failing tests for demo files.

	2023.11.27	v3.3

	1. Add missing void to test function args in type declaration.
	2. Cleanup whitespaces.

	2023.11.26	v3.2

	1. Fix "missing sentinel in function call" warning in execl().
	   Looks like when compiled using gcc with new C standard the
	   warning is printed because last value to excel() has to be
	   a null terminator.  It was as I was using value of 0.  For
	   compiler to not complain it has to be casted to (char*).
	   This is what NULL macro does so I just changed 0 to NULL.
	2. Extend documentation with table of contents and change log.
	   Recently I feel like relaying on version control with such
	   information like changelog taken from commit messages might
	   not be a good idea for a project like this as we expect to
	   copy and paste single file to other projects.  Then copy it
	   from one project to another.  Connection to original repo
	   might get lost very quickly even tho link to it is provided
	   at the very top.  I recently copiec similar single header
	   libs from other repositories and I have no idea where those
	   come from.  So I will try to maintain changelog here.

	2023.10.31	v3.1

	1. Cast arguments to strings in string macros.
	2. Calculate _wh_eq offset in more readable way.
	3. Wrap strings in quotes in _wh_eq error message.
	4. Leave a TODO comment about arrow _wh_eq in error message.
	5. Remove unused sum variable.
	6. Fix typo in error messages of _wh_srun.
*/

#ifdef _WALTER_H
#error "walter.h can be included only once (read doc)"
#endif
#define _WALTER_H

#include <assert.h>
#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define WH_MAX  256             /* Maximum number of TEST/SKIP/ONLY */
#define WH_SHOW 32              /* How many chars print on error    */

#define __WH_CORE(_msg, _id, _line, _type)                           \
	void __wh_test_body##_id();                                  \
	/* This function will run before main() function. */         \
	void __wh_test##_id() __attribute__ ((constructor));         \
	void __wh_test##_id() {                                      \
		/* Init on first TEST/SKIP/ONLY(). */                \
		if (_wh.all == 0) _wh.fname = __FILE__;              \
		if (_type == _WH_ONLY) _wh.only = 1;                 \
		_wh.fn[_wh.all] = &__wh_test_body##_id;              \
		_wh.msg[_wh.all] = _msg;                             \
		_wh.line[_wh.all] = _line;                           \
		_wh.type[_wh.all] = _type;                           \
		_wh.all++;                                           \
		if (_wh.all <= WH_MAX) return;                       \
		errx(1, "Exceeded WH_MAX (read documentation)");     \
	}                                                            \
	void __wh_test_body##_id()

/* Intermediate macro function _WH_CORE is necessary to "unwrap"
 * __LINE__ macro so it could be used as a ID string. */
#define _WH_CORE(msg, id, type) __WH_CORE(msg, id, __LINE__, type)

/* Main test macros for defining test blocks. */
#define TEST(msg) _WH_CORE(msg, __LINE__, _WH_TEST)
#define SKIP(msg) _WH_CORE(msg, __LINE__, _WH_SKIP)
#define ONLY(msg) _WH_CORE(msg, __LINE__, _WH_ONLY)

/* Core assertion macro that every other assertion macro use. */
#define _WH_ASSERT(_bool, _msg, _line) do {                          \
		_wh.last_all++;                                      \
		if ((_bool)) break;             /* Pass */	     \
		_wh.last_fail++;                /* Fail */           \
		fprintf(stderr, "%s%s:%d:\t%s\n",                    \
			_wh.cwd, _wh.fname, _line, _msg);            \
		if (_wh.quick) return;          /* End quick */      \
	} while(0)
#define ASSERT(a,msg) _WH_ASSERT(a, msg, __LINE__)

/* Basic assertions. */
#define OK(a)      ASSERT((a), "OK("#a")")
#define EQ(a,b,n)  ASSERT(_wh_eq(1,a,b,n,n), "EQ("#a", "#b", "#n")")
#define NEQ(a,b,n) ASSERT(_wh_eq(0,a,b,n,n), "NEQ("#a", "#b", "#n")")
#define SEQ(a,b)   ASSERT(_wh_seq(1,(char*)a,(char*)b), "SEQ("#a", "#b")")
#define SNEQ(a,b)  ASSERT(_wh_seq(0,(char*)a,(char*)b), "SNEQ("#a", "#b")")

/* Run CMD assertions. */
#define RUN(cmd, in, out, err, code)                                 \
	ASSERT(_wh_run(cmd, in, out, err, code),                     \
	       "RUN("#cmd", "#in", "#out", "#err", "#code")")
#define S2F(str) _wh_s2f(str)

enum {                          /* Test macro types */
	_WH_TEST = 0,           /* Regular test */
	_WH_SKIP,               /* Skip test */
	_WH_ONLY,               /* Run only ONLY tests */
	__WH_SIZ                /* For _wh_type array size */
};
struct {                        /* Tests global state */
	char    cwd[4096];      /* Current working directory */
	char   *fname;          /* Test source file name */
	int     verbose;	/* True for verbose mode -v */
	int     quick;          /* True for quick mode -q */
	int     fast;           /* True for fast mode -f */
	int     only;           /* True when ONLY() macro was used */
	int     all;            /* Number of all tests */
	int     fail;           /* Number of failed tests */
	int     last_all;       /* Last test assertions count */
	int     last_fail;      /* Last test assertions fails count */
	int     line[WH_MAX];   /* TEST macros line in file */
	int     type[WH_MAX];   /* Test type enum _WH_SKIP... */
	char   *msg[WH_MAX];    /* TEST macro messages */
	void  (*fn[WH_MAX])();  /* Test functions pointers */
} _wh = {0};

/* String representations of test macro types. */
char *_wh_type[__WH_SIZ] = {"TEST", "SKIP", "ONLY"};

/* Help message (-h). */
char *_wh_help =
"usage: %s [options]\n"
"\n"
"options:\n"
"	-v	Print verbose output.\n"
"	-q	Quick, stop TEST on first failed assertion.\n"
"	-f	Fast, exit program on first failed TEST.\n"
"	-a	Print absolute file paths.\n"
"	-h	Prints this help message.\n";

/* Compare buffers BUF0 and BUF1 of SIZ size.  Return non 0 value when
 * EQ value is 1 and buffers are the same, or when EQ value is 0 and
 * buffers are different. */
int _wh_eq(int eq, char *buf0, char *buf1, size_t siz0, size_t size1);

/* Compare strings STR0 and STR1.  Return non 0 value when EQ value is
 * 1 and strings are the same, or when EQ value is 0 and strings are
 * different. */
int _wh_seq(int eq, char *str0, char *str1);

/* Compare files pointed by FD0 and FD1 file descriptors.  Print error
 * message showing where the difference is if content is not the same.
 * Return 0 when files are the same. */
int _wh_fdcmp(int fd0, int fd1);

/* Test CMD.  IN, OUT and ERR are optional paths to files used as
 * stdin, stdou and stderr, can be omitted by setting them to NULL.
 * Function will run CMD command with IN file content if given and
 * test if stdout is equal to content of OUT file if given, same for
 * ERR and will compare CODE expected program exit code with actually
 * CMD exit code.  Return 0 on failure. */
int _wh_run(char *cmd, char *in, char *out, char *err, int code);

/* Return random static string of LEN length. */
char *_wh_rand(int len);

/* Creates tmp file the mkstemp way.  Path to temporary file will be
 * created in NAME buffer as NULL terminated string.  On success file
 * descriptor is returned, terminate program on error. */
int _wh_tmpf(char *name);

/* String To File.  Create temporary file with content of STR string.
 * Return path to that file. */
char *_wh_s2f(char *str);

/* Runs tests defined with TEST, SKIP and ONLY macros.
 * Return number of failed tests or 0 on success. */
int
main(int argc, char **argv)
{
	int i;
	while ((i = getopt(argc, argv, "vqfah")) != -1) {
		switch (i) {
		case 'v': _wh.verbose = 1; break;
		case 'q': _wh.quick = 1; break;
		case 'f': _wh.fast = 1; break;
		case 'a':
			/* -2 to reserve space for '/' and '\0'. */
			if (!getcwd(_wh.cwd, sizeof(_wh.cwd)-2)) {
				err(1, "getcwd");
			}
			strcat(_wh.cwd, "/");
			break;
		case 'h':
		default:
			fprintf(stderr, _wh_help, argv[0]);
			return 1;
		}
	}
	/* Run tests.  Print error on fail. */
	for (i = 0; i < _wh.all; i++) {
		if ((_wh.only) && _wh.type[i] != _WH_ONLY) {
			continue;
		}
		if (_wh.type[i] == _WH_SKIP) {
			fprintf(stderr, "%s%s:%d:\tSKIP %s\n",
				_wh.cwd,
				_wh.fname,
				_wh.line[i],
				_wh.msg[i]);
			continue;
		}
		_wh.last_all = 0;
		_wh.last_fail = 0;
		(*_wh.fn[i])();	/* Run test, print assert fails */
		if (_wh.last_fail) {
			_wh.fail++;
			fprintf(stderr, "%s%s:%d:\t%s %s\n",
				_wh.cwd,
				_wh.fname,
				_wh.line[i],
				_wh_type[_wh.type[i]],
				_wh.msg[i]);
		}
		if (_wh.verbose) {
			fprintf(stdout, "%s %s\t(%d/%d) pass\n",
				_wh_type[_wh.type[i]],
				_wh.msg[i],
				_wh.last_all - _wh.last_fail,
				_wh.last_all);
		}
		if (_wh.fast && _wh.last_fail) {
			break;
		}
	}
	/* Print verbose summary or errors if occurred. */
	if (_wh.verbose) {
		fprintf(stdout, "FILE %s%s\t(%d/%d) pass\n",
			_wh.cwd, _wh.fname, _wh.all - _wh.fail, _wh.all);
	} else if (_wh.fail) {
		fprintf(stderr, "%s%s\t%d fail\n",
			_wh.cwd, _wh.fname, _wh.fail);
	}
	return _wh.fail;
}

int
_wh_eq(int eq, char *buf0, char *buf1, size_t siz0, size_t siz1)
{
	size_t i=0, offset;
	if (!buf0 || !buf1) {	/* Handle nulls */
		if (eq == (buf0 == buf1)) {
			return 1;
		}
	} else {
		while (i<siz0 && i<siz1 && buf0[i] == buf1[i]) i++;
		if (eq == (buf0[i-1] == buf1[i-1] && siz0 == i && siz1 == i)) {
			return 1;
		}
	}
	offset = i - (i % WH_SHOW);
	buf0 += offset;
	buf1 += offset;
	siz0 -= offset;
	siz1 -= offset;
	if (siz0 > WH_SHOW) siz0 = WH_SHOW;
	if (siz1 > WH_SHOW) siz1 = WH_SHOW;
	fprintf(stderr,
		"\t First incorrect byte: %lu\n"
		"\t\"%.*s\"\n"
		"\t\"%.*s\"\n",
		i,
		(int)siz0, buf0 ? buf0 : "<NULL>",
		(int)siz1, buf1 ? buf1 : "<NULL>");
	return 0;
}

int
_wh_seq(int eq, char *str0, char *str1)
{
	size_t siz0 = str0 ? strlen(str0) : 0;
	size_t siz1 = str1 ? strlen(str1) : 0;
	return _wh_eq(eq, str0, str1, siz0, siz1);
}

int
_wh_fdcmp(int fd0, int fd1)
{
	ssize_t siz0, siz1;     /* Size of read buffer */
	char buf0[BUFSIZ];      /* Buffer for reading from fd0 */
	char buf1[BUFSIZ];      /* Buffer for reading from fd1 */
	while ((siz0 = read(fd0, buf0, BUFSIZ)) > 0) {
		/* We should be able to read the same amount of bytes
		 * SIZ0 from FD1 as we read from FD0.  So we should at
		 * least be able to read from FD1 and SIZ0 and SIZ1
		 * should be the same. */
		if ((siz1 = read(fd1, buf1, siz0)) == -1) {
			if (!_wh_eq(1, buf0, 0, siz0, 0)) {
				return 1;
			}
			/* Preceding condition should always be true.
			 * But just in case to catch unforeseen edge
			 * case I have this message. */
			warnx("Unreachable %d\n", __LINE__);
			return 1;
		}
		if (!_wh_eq(1, buf0, buf1, siz0, siz1)) {
			return 1;
		}
	}
	/* At this point BUF0 was read in it's entirely but BUF1 might
	 * still hold more data.  If difference was not found at this
	 * point then check if there is still something in BUF1 which
	 * means buffers are different at last position of BUF0. */
	if ((siz1 = read(fd1, buf1, BUFSIZ)) > 0) {
		if (!_wh_eq(1, 0, buf1, 0, siz1)) {
			return 1;
		}
		/* Preceding condition should always be true.  But
		 * just in case to catch unforeseen edge case I have
		 * this message. */
		warnx("Unreachable %d\n", __LINE__);
		return 1;
	}
	return 0;
}

int
_wh_run(char *cmd, char *sin, char *sout, char *serr, int code)
{
	int fd;                 /* sin, sout, serr file descriptor */
	int fd_in[2];           /* File descriptors for input pipe */
	int fd_out[2];          /* File descriptors for output pipe */
	int fd_err[2];          /* File descriptors for error pipe */
	int ws, wes;            /* Wait status, wait exit status */
	pid_t pid;              /* CMD process id */
	char buf[BUFSIZ];       /* For passing stdin to PID SIN pipe */
	ssize_t ssiz;           /* Signed size, to read bytes */
	/* Open pipes.  We need pipe for stdin, stdout and stderr. */
	if (pipe(fd_in)  == -1) err(1, "pipe(fd_in)");
	if (pipe(fd_out) == -1) err(1, "pipe(fd_out)");
	if (pipe(fd_err) == -1) err(1, "pipe(fd_err)");
	/* Fork process to run CMD as a child. */
	if ((pid = fork()) == -1) err(1, "fork");
	if (pid == 0) {           /* Child process, the CMD */
		close(fd_in[1]);  /* Index 1 is for writing. */
		close(fd_out[0]); /* Index 0 is for reading */
		close(fd_err[0]);
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		dup(fd_in[0]);
		dup(fd_out[1]);
		dup(fd_err[1]);
		if (execl("/bin/sh", "sh", "-c", cmd, NULL) == -1) {
			/* TODO(irek): Probably a bug, how it supposed
			 * to report the error when being the child
			 * process and the return 0 should probably be
			 * replaced with exit(1). */
			perror("execl");
			return 0;
		}
		close(fd_in[0]); close(fd_out[1]); close(fd_err[1]);
		exit(0);
	}
	/* Parent process. */
	close(fd_in[0]);
	close(fd_out[1]);
	close(fd_err[1]);
	/* Pass standard input. */
	if (sin) {
		if ((fd = open(sin, O_RDONLY)) == -1) {
			err(1, "open(%s)", sin);
		}
		while ((ssiz = read(fd, buf, BUFSIZ)) > 0) {
			if (write(fd_in[1], buf, ssiz) == -1) {
				perror("write(sin)");
				return 0;
			}
		}
		if (close(fd) == -1) {
			err(1, "close(%s)", sin);
		}
	}
	close(fd_in[1]);
	/* Compare standard output. */
	if (sout) {
		if ((fd = open(sout, O_RDONLY)) == -1) {
			err(1, "open(%s)", sout);
		}
		if (_wh_fdcmp(fd_out[0], fd)) {
			fprintf(stderr, "\tstdout\n");
			return 0;
		}
		if (close(fd) == -1) {
			err(1, "close(%s)", sout);
		}
	}
	close(fd_out[0]);
	/* Compare standard error. */
	if (serr) {
		if ((fd = open(serr, O_RDONLY)) == -1) {
			err(1, "open(%s)", serr);
		}
		if (_wh_fdcmp(fd_err[0], fd)) {
			fprintf(stderr, "\tstderr\n");
			return 0;
		}
		if (close(fd) == -1) {
			err(1, "close(%s)", serr);
		}
	}
	close(fd_err[0]);
	/* Wait for child process to exit. */
	if (waitpid(pid, &ws, 0) == -1) {
		perror("waitpid");
		return 0;
	}
	/* Get child process exit code. */
	if (WIFEXITED(ws) && (wes = WEXITSTATUS(ws)) != code) {
		fprintf(stderr, "\tExpected exit code %d, got %d\n",
			code, wes);
		return 0;       /* Failed */
	}
	return 1;               /* Success */
}

char *
_wh_rand(int len)
{
	static char *POOL = "abcdefghijklmnoprstuwxyz0123456789";
	static int seed = 0;
	static char str[32];
	assert(len > 3);
	assert(len < (int)sizeof str -1);
	srand(time(0) + seed++);
	str[len] = 0;
	while (len--) {
		str[len] = POOL[rand() % (sizeof POOL -1)];
	}
	return str;
}

int
_wh_tmpf(char *name)
{
	static char *PREFIX = "/tmp/walter";
	int fd;
	do {
		sprintf(name, "%s%s", PREFIX, _wh_rand(6));
	} while (!access(name, F_OK));
	if ((fd = open(name, O_RDWR | O_CREAT | O_EXCL)) == -1) {
		err(1, "open(%s)", name);
	}
	return fd;
}

char *
_wh_s2f(char *str)
{
	char *name = malloc(FILENAME_MAX);	/* No free() */
	int fd = _wh_tmpf(name);
	assert(str);
	if (write(fd, str, strlen(str)) == -1) {
		err(1, "write(%s)", name);
	}
	if (close(fd) == -1) {
		err(1, "close(%s)", name);
	}
	return name;
}

/* Licenses:
This software is available under 2 licenses, choose one.

--------------------------------------------------------------------------

(1) MIT License, Copyright 2025 Irek

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

--------------------------------------------------------------------------

(2) Public Domain

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or distribute
this software, either in source code form or as a compiled binary,
for any purpose, commercial or non-commercial, and by any means.

In jurisdictions that recognize copyright laws, the author or authors of
this software dedicate any and all copyright interest in the software to
the public domain.  We make this dedication for the benefit of the public
at large and to the detriment of our heirs and successors.  We intend
this dedication to be an overt act of relinquishment in perpetuity of
all present and future rights to this software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <https://unlicense.org/>
*/
