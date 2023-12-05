/* Walter is a single header library for writing unit tests in C made
 * with fewer complications by avoiding boilerplate.
 *
 * walter.h v4.0 from https://github.com/ir33k/walter by irek@gabr.pl
 *
 * Table of contents:
 *
 *	Example test file
 *	Compile and run
 *	Disclaimers
 *	Change log
 *	Licenses (at the very end of this file)
 *
 * Example test file:
 *
 *	// File: demo.t.c
 *	#define WH_MAX 1024             // Optional, number of tests
 *	#include "walter.h"             // Include test lib
 *
 *	TEST("Test message")            // Define test with assertions
 *	{
 *		// Basic assertions
 *		ASSERT(bool, "msg");    // Print message on false
 *		OK(bool);               // Is boolean true?
 *		EQ(b1, b2, size);       // Are buffers equal?
 *		NEQ(b1, b2, size);      // Are buffers not equal?
 *		SEQ(s1, s2);            // Are strings equal?
 *		SNEQ(s1, s2);           // Are strings not equal?
 *		END();                  // End test here
 *
 *		// Run CMD with IN standard input expecting OUT
 *		// standard output and ERR standard error and CODE
 *		// exit code.  When file path for IN, OUT, ERR are
 *		// NULL then this part of input/output is ignored.
 *		//
 *		//   CMD          IN        OUT        ERR       CODE
 *		RUN("grep wh_",  "in.txt", "out.txt", "err.txt", 0);
 *		RUN("sed -i",    "in.txt",  NULL,      NULL,     1);
 *		RUN("./demo0.t",  NULL,    "out.txt", "err.txt", 5);
 *		RUN("ls -lh",     NULL,    "out.txt",  NULL,     0);
 *		RUN("pwd",        0,        0,         0,        0);
 *
 *		// If you want to use string literal instead of file
 *		// path then use S2F() macro as argument that creates
 *		// temporary file with provided string.
 *		RUN("tr abc 123", S2F("AaBbCc"), S2F("A1B2C3"), 0, 0);
 *	}
 *	TEST("Another test 1") { ... }  // Define as many as WH_MAX
 *	SKIP("Another test 2") { ... }  // Skip or just ignore test
 *	SKIP("Another test 3") {}       // Body can be empty
 *	SKIP("TODO test 4")    {}       // Can be used for TODOs
 *	ONLY("Another test 5") { ... }  // Ignore all other tests
 *
 *	// There is no "main()" function.
 *
 * Compile and run:
 *
 *	$ cc -o demo.t demo.t.c   # Compile
 *	$ ./dmeo.t -h             # Print usage help
 *	$ ./dmeo.t                # Run tests
 *
 * Disclaimers:
 *
 *	Library can be included only in one test program because it
 *	has single global tests state, it defines it's own "main"
 *	function and TEST macro relays on file line numbers.
 *
 *	By default you can define only 64 tests but this can be
 *	changed by predefining WH_MAX (see example).  Variables,
 *	functions and macros not mentioned in example test program
 *	should not be used.
 *
 *	WH_ prefix stands for Walter.H.  _WH_ is used for private
 *	stuff.  __WH_ is for super epic internal private stuff, just
 *	move along, this is not the code you are looking for \(-_- )
 *
 * Change Log:
 *
 *	2023.12.04	v4.0
 *
 *	1. Major version update because of breaking changes in SRUN().
 *	2. Remove SRUN() in favor of RUN() used with S2F().
 *	3. Implement S2F() macro and adjust documentation accordingly.
 *	4. Fix failing tests for demo files.
 *
 *	2023.11.27	v3.3
 *
 *	1. Add missing void to test function args in type declaration.
 *	2. Cleanup whitespaces.
 *
 *	2023.11.26	v3.2
 *
 *	1. Fix "missing sentinel in function call" warning in execl().
 *	   Looks like when compiled using gcc with new C standard the
 *	   warning is printed because last value to excel() has to be
 *	   a null terminator.  It was as I was using value of 0.  For
 *	   compiler to not complain it has to be casted to (char*).
 *	   This is what NULL macro does so I just changed 0 to NULL.
 *	2. Extend documentation with table of contents and change log.
 *	   Recently I feel like relaying on version control with such
 *	   information like changelog taken from commit messages might
 *	   not be a good idea for a project like this as we expect to
 *	   copy and paste single file to other projects.  Then copy it
 *	   from one project to another.  Connection to original repo
 *	   might get lost very quickly even tho link to it is provided
 *	   at the very top.  I recently copiec similar single header
 *	   libs from other repositories and I have no idea where those
 *	   come from.  So I will try to maintain changelog here.
 *
 *	2023.10.31	v3.1
 *
 *	1. Cast arguments to strings in string macros.
 *	2. Calculate _wh_eq offset in more readable way.
 *	3. Wrap strings in quotes in _wh_eq error message.
 *	4. Leave a TODO comment about arrow _wh_eq in error message.
 *	5. Remove unused sum variable.
 *	6. Fix typo in error messages of _wh_srun.
 *
 */
#ifdef _WALTER_H
#error "walter.h can't be included multiple times (read doc)"
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

#ifndef WH_MAX                  /* Maximum number of TEST/SKIP/ONLY */
#define WH_MAX  64              /* test macros that can be handled. */
#endif                          /* Predefine to handle more tests.  */

#ifndef WH_SHOW                 /* How many characters/bytes print  */
#define WH_SHOW 32              /* when file comperation fail, RUN. */
#endif                          /* Predefine for different amount.  */

#define __WH_CORE(_msg, _id, _line, _type)                           \
	void __wh_test_body##_id(void);                              \
	/* This function will run before "main" function. */         \
	void __wh_test##_id(void) __attribute__ ((constructor));     \
	void __wh_test##_id(void) {                                  \
		/* Init on first TEST(). */                          \
		if (_wh.all == 0) _wh.fname = __FILE__;              \
		if (_type == _WH_ONLY) _wh.flag |= _WH_O;            \
		_wh.fn[_wh.all] = &__wh_test_body##_id;              \
		_wh.msg[_wh.all] = _msg;                             \
		_wh.line[_wh.all] = _line;                           \
		_wh.type[_wh.all] = _type;                           \
		_wh.all++;                                           \
		if (_wh.all <= WH_MAX) return;                       \
		fprintf(stderr, "ERR exceeded WH_MAX (see doc)\n");  \
		exit(1);        /* Too many tests */                 \
	}                                                            \
	void __wh_test_body##_id(void)

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
		if ((_bool)) break;             /* Pass */           \
		_wh.last_err++;                 /* Fail */           \
		fprintf(stderr, "%s:%d:\t%s\n",                      \
			_wh.fname, _line, _msg);                     \
		if (_wh.flag & _WH_Q) return;   /* End quick */      \
	} while(0)
#define ASSERT(a,msg) _WH_ASSERT(a, msg, __LINE__)

/* Basic assertions. */
#define OK(a)      ASSERT((a), "OK("#a")")
#define EQ(a,b,n)  ASSERT(_wh_eq(1,a,b,n,n), "EQ("#a", "#b", "#n")")
#define NEQ(a,b,n) ASSERT(_wh_eq(0,a,b,n,n), "NEQ("#a", "#b", "#n")")
#define SEQ(a,b)   ASSERT(_wh_seq(1,(char*)a,(char*)b), "SEQ("#a", "#b")")
#define SNEQ(a,b)  ASSERT(_wh_seq(0,(char*)a,(char*)b), "SNEQ("#a", "#b")")

/* Run CMD assertions. */
#define RUN(cmd, in, out, err, code)                            \
	ASSERT(_wh_run(cmd, in, out, err, code),                \
	       "RUN("#cmd", "#in", "#out", "#err", "#code")")
#define S2F(str) _wh_s2f(str)

/* Force end of test block. */
#define END() do {return;} while(0)

enum {                          /* Flags */
	_WH_V = 1,              /* Verbose mode */
	_WH_Q = 2,              /* Quick mode */
	_WH_F = 4,              /* Fast mode */
	_WH_O = 8               /* ONLY test macro was used */
};
enum {                          /* Test macro types */
	_WH_TEST = 0,           /* Regular test */
	_WH_SKIP,               /* Skip test */
	_WH_ONLY,               /* Run only ONLY tests */
	__WH_SIZ                /* For _wh_type array size */
};
struct {                        /* Tests global state */
	char  *fname;           /* Test source file name */
	int    flag;            /* Program flags, enum _WH_V... */
	int    all;             /* Number of all tests */
	int    err;             /* Number of failed tests */
	int    last_all;        /* Last test assertions count */
	int    last_err;        /* Last test assertions fails */
	int    line[WH_MAX];    /* TEST macros line in file */
	int    type[WH_MAX];    /* Test type enum WH_SKIP... */
	char  *msg[WH_MAX];     /* TEST macro messages */
	void (*fn[WH_MAX])(void); /* Test functions pointers */
} _wh = {0};

/* String representations of test macro types. */
const char *_wh_type[__WH_SIZ] = {"TEST", "SKIP", "ONLY"};

/* Compare buffers BUF0 and BUF1 of SIZ size.  Return non 0 value when
 * EQ value is 1 and buffers are the same, or when EQ value is 0 and
 * buffers are different. */
int _wh_eq(int eq, char *buf0, char *buf1, size_t siz0, size_t size1);

/* Compare strings STR0 and STR1.  Return non 0 value when EQ value is
 * 1 and strings are the same, or when EQ value is 0 and strings are
 * different. */
int _wh_seq(int eq, char *str0, char *str1);

/* Compare files pointed by FD0 and FD1 file desciptors.  Print error
 * message showing where the differance is if content is not the
 * same.  Return 0 when files are the same. */
int _wh_fdcmp(int fd0, int fd1);

/* Test CMD.  IN, OUT and ERR are optional paths to files used as
 * stdin, stdou and stderr, can be ommited by setting them to NULL.
 * Function will run CMD command with IN file content if given and
 * test if stdout is equal to content of OUT file if given, same for
 * ERR and will compare CODE expected program exit code with actuall
 * CMD exit code.  Return 0 on failure. */
int _wh_run(char *cmd, char *in, char *out, char *err, int code);

/* Return random static string of LEN length. */
char *_wh_rand(int len);

/* Creates tmp file the mkstemp way.  Path to temporary file will be
 * created in NAME buffer as NULL terminated string.  On success file
 * desciptor is returned, terminate program on error. */
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
	while ((i = getopt(argc, argv, "vqfh")) != -1) {
		switch (i) {
		case 'v': _wh.flag |= _WH_V; break;
		case 'q': _wh.flag |= _WH_Q; break;
		case 'f': _wh.flag |= _WH_F; break;
		case 'h':
		default:
			fprintf(stderr, "usage: %s [-vqh]\n\n"
				"\t-v\tPrint verbose output.\n"
				"\t-q\tQuick, stop test on first"
				" failed assertion.\n"
				"\t-f\tFast, exit program on first"
				" failed test.\n"
				"\t-h\tPrints this help message.\n",
				argv[0]);
			return 1;
		}
	}
	/* Run tests.  Print error on fail. */
	for (i = 0; i < _wh.all; i++) {
		if ((_wh.flag & _WH_O) && _wh.type[i] != _WH_ONLY) {
			continue;
		}
		if (_wh.type[i] == _WH_SKIP) {
			fprintf(stderr, "%s:%d:\tSKIP %s\n",
				_wh.fname, _wh.line[i], _wh.msg[i]);
			continue;
		}
		_wh.last_all = 0;
		_wh.last_err = 0;
		(*_wh.fn[i])();	/* Run test, print assert fails */
		if (_wh.last_err) {
			_wh.err++;
			fprintf(stderr, "%s:%d:\t%s %s\n",
				_wh.fname,
				_wh.line[i],
				_wh_type[_wh.type[i]],
				_wh.msg[i]);
		}
		if (_wh.flag & _WH_V) {
			fprintf(stdout, "%s %s\t(%d/%d) pass\n",
				_wh_type[_wh.type[i]],
				_wh.msg[i],
				_wh.last_all - _wh.last_err,
				_wh.last_all);
		}
		if (_wh.flag & _WH_F && _wh.last_err) {
			break;
		}
	}
	/* Print verbose summary or errors if occured. */
	if (_wh.flag & _WH_V) {
		fprintf(stdout, "FILE %s\t(%d/%d) pass\n", _wh.fname,
			_wh.all - _wh.err, _wh.all);
	} else if (_wh.err) {
		fprintf(stderr, "%s\t%d err\n", _wh.fname, _wh.err);
	}
	return _wh.err;
}

int
_wh_eq(int eq, char *buf0, char *buf1, size_t siz0, size_t siz1)
{
	size_t i=0, offset;
	if (!buf0 || !buf1) {	/* Handle nulls */
		if ((buf0 == buf1) == eq) {
			return 1;
		}
	} else {
		while (i<siz0 && i<siz1 && buf0[i] == buf1[i]) i++;
		if ((buf0[i-1] == buf1[i-1] && siz0 == i && siz1 == i) == eq) {
			return 1;
		}
	}
	offset = i - (i % WH_SHOW);
	buf0 += offset;
	siz0 -= offset;
	buf1 += offset;
	siz1 -= offset;
	fprintf(stderr,
		"\t %*s byte %lu\n"
		"\t\"%.*s\"\n"
		"\t\"%.*s\"\n",
		/* TODO(irek): This arrow to invalid character is not
		 * very precise.  It's because if there will be white
		 * character like \r or \t then it's impossible for me
		 * to know where to put this arrow.  So maybe to avoid
		 * confusions I should just have information about
		 * byte index? */
		(int)(i-offset)+1, "v", i,
		(int)(WH_SHOW < siz0 ? WH_SHOW : siz0), buf0 ? buf0 : "<NULL>",
		(int)(WH_SHOW < siz1 ? WH_SHOW : siz1), buf1 ? buf1 : "<NULL>");
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
		 * leas be able to read from FD1 and SIZ0 and SIZ1
		 * should be the same. */
		if ((siz1 = read(fd1, buf1, siz0)) == -1) {
			if (!_wh_eq(1, buf0, 0, siz0, 0)) {
				return 1;
			}
			/* Preceding condition should always be true.
			 * But just in case to catch unforeseen edge
			 * case I have this message. */
			fprintf(stderr, "ERROR: unreachable %d\n", __LINE__);
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
		fprintf(stderr, "ERROR: unreachable %d\n", __LINE__);
		return 1;
	}
	return 0;
}

int
_wh_run(char *cmd, char *sin, char *sout, char *serr, int code)
{
	int fd;                 /* sin, sout, serr file desciptor */
	int fd_in[2];           /* File desciptors for input pipe */
	int fd_out[2];          /* File desciptors for output pipe */
	int fd_err[2];          /* File desciptors for error pipe */
	int ws, wes;            /* Wait status, wait exit status */
	pid_t pid;              /* CMD process id */
	char buf[BUFSIZ];       /* For passing stdin to PID SIN pipe */
	ssize_t ssiz;           /* Signed size, to read bytes */
	/* Open pipes.  We need pipe for stdin, stdout and asderr. */
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
	/* Pass standard intput. */
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
	static int seed = 0;
	static const char *allow =
		"ABCDEFGHIJKLMNOPRSTUWXYZ"
		"abcdefghijklmnoprstuwxyz"
		"0123456789";
	size_t limit = strlen(allow);
	static char str[32];
	assert(len < 32);
	srand(time(0) + seed++);
	str[len] = 0;
	while (len--) {
		str[len] = allow[rand() % limit];
	}
	return str;
}

int
_wh_tmpf(char *name)
{
	static const char *prefix = "/tmp/walter";
	int fd;
	do {
		sprintf(name, "%s%s", prefix, _wh_rand(6));
	} while (!access(name, F_OK));
	if ((fd = open(name, O_RDWR | O_CREAT | O_EXCL)) == -1) {
		err(1, "open(%s)", name);
	}
	return fd;
}

char *
_wh_s2f(char *str)
{
	char *name = malloc(FILENAME_MAX);
	int fd = _wh_tmpf(name);
	if (write(fd, str, strlen(str)) == -1) {
		err(1, "write(%s)", name);
	}
	if (close(fd) == -1) {
		err(1, "close(%s)", name);
	}
	return name;
}
/* Licenses:
This software is available under 2 licenses, choose whichever.
----------------------------------------------------------------------
ALTERNATIVE A - MIT License, Copyright (c) 2023 Irek
Permission is hereby granted, free  of charge, to any person obtaining
a  copy  of this  software  and  associated documentation  files  (the
"Software"), to  deal in  the Software without  restriction, including
without limitation  the rights to  use, copy, modify,  merge, publish,
distribute, sublicense,  and/or sell  copies of  the Software,  and to
permit persons to whom the Software  is furnished to do so, subject to
the  following  conditions:  The   above  copyright  notice  and  this
permission  notice shall  be  included in  all  copies or  substantial
portions of the  Software.  THE SOFTWARE IS PROVIDED  "AS IS", WITHOUT
WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE WARRANTIES  OF MERCHANTABILITY,  FITNESS FOR A  PARTICULAR PURPOSE
AND  NONINFRINGEMENT.  IN  NO  EVENT SHALL  THE  AUTHORS OR  COPYRIGHT
HOLDERS BE LIABLE  FOR ANY CLAIM, DAMAGES OR  OTHER LIABILITY, WHETHER
IN AN ACTION  OF CONTRACT, TORT OR OTHERWISE, ARISING  FROM, OUT OF OR
IN CONNECTION  WITH THE SOFTWARE OR  THE USE OR OTHER  DEALINGS IN THE
SOFTWARE.
----------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This  is  free and  unencumbered  software  released into  the  public
domain.  Anyone is free to  copy, modify, publish, use, compile, sell,
or  distribute this  software,  either in  source code  form  or as  a
compiled binary, for any purpose, commercial or non-commercial, and by
any means.  In jurisdictions that recognize copyright laws, the author
or authors of this software dedicate any and all copyright interest in
the software  to the public domain.   We make this dedication  for the
benefit of the public  at large and to the detriment  of our heirs and
successors.   We  intend  this  dedication  to  be  an  overt  act  of
relinquishment in perpetuity of all  present and future rights to this
software  under copyright  law.   THE SOFTWARE  IS  PROVIDED "AS  IS",
WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR IMPLIED, INCLUDING  BUT NOT
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT.  IN NO  EVENT SHALL THE AUTHORS BE LIABLE
FOR ANY  CLAIM, DAMAGES OR  OTHER LIABILITY,  WHETHER IN AN  ACTION OF
CONTRACT, TORT  OR OTHERWISE,  ARISING FROM, OUT  OF OR  IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
