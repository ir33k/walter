/* Walter is a single header library for writing unit tests in C made
 * with fewer complications by avoiding boilerplate.
 *
 * walter.h v3.0 from https://github.com/ir33k/walter by irek@gabr.pl
 *
 * Example usage:
 *
 *	// File: demo.t.c
 *	#define WH_MAX 1024             // Optional, number of tests
 *	#include "walter.h"             // Include test lib
 *
 *	TEST("Test message")            // Define test with assertions
 *	{
 *		// Basic assertions
 *		OK(bool);               // Is boolean true?
 *		EQ(a, b);               // Are values equal?
 *		ASSERT(bool, "msg");    // Print message on false
 *		STR_EQ(s1, s2);         // Are strings equal?
 *		BUF_EQ(b1, b2, size);   // Are buffers equal?
 *		END();                  // End test here
 *		STR_NEQ(s1, s2);        // Are strings not equal?
 *		BUF_NEQ(b1, b2, size);  // Are buffers not equal?
 *
 *		// Run CMD with IN standard input expecting OUT
 *		// standard output and ERR standard error and CODE
 *              // exit code.  When file path for IN, OUT, ERR are
 *		// NULL then this part of input/output is ignored.
 *		//
 *		//   CMD          IN        OUT        ERR       CODE
 *		RUN("grep wh_",  "in.txt", "out.txt", "err.txt", 0);
 *		RUN("sed -i",    "in.txt",  NULL,      NULL,     1);
 *		RUN("./demo0.t",  NULL,    "out.txt", "err.txt", 5);
 *		RUN("ls -lh",     NULL,    "out.txt",  NULL,     0);
 *		RUN("pwd",        0,        0,         0,        0);
 *
 *		// Same as RUN macro but provide string directly
 *		// instead of paths to files with expected stdin,
 *		// stdout and stderr.  All 3 values can be omitted
 *		// with NULL.
 *		STR_RUN("tr abc 123", "AaBbCc", "A1B2C3", 0, 0);
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
 * Can be included only in one test program because it has single
 * global tests state, it defines it's own "main" function and TEST
 * macro relays on file line numbers.
 *
 * By default You can define only 64 tests but this can be changed by
 * predefining WH_MAX (see example).  Variables, functions and macros
 * not mentioned in example test program should not be used.
 *
 * WH_ prefix stands for Walter.H.  _WH_ is used for private stuff.
 * __WH_ is for super epic internal private stuff, just move along,
 * this is not the code you are looking for  \(-_- )
 */
#ifdef _WALTER_H
#error "walter.h can't be included multiple times (read doc)"
#endif
#define _WALTER_H

#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <err.h>

#ifndef WH_MAX			/* Maximum number of TEST/SKIP/ONLY */
#define WH_MAX  64		/* test macros that can be handled. */
#endif				/* Predefine to handle more tests.  */

#ifndef WH_SHOW			/* How many characters/bytes print  */
#define WH_SHOW 32		/* when file comperation fail, RUN. */
#endif				/* Predefine for different amount.  */

#define WH_IN	"/tmp/walter.h.in"
#define WH_OUT	"/tmp/walter.h.out"
#define WH_ERR	"/tmp/walter.h.err"

#define __WH_BASE(_msg, id, _line, _type)                            \
	void __wh_test_body##id(void);                               \
	/* This function will run before "main" function. */         \
	void __wh_test##id(void) __attribute__ ((constructor));      \
	void __wh_test##id(void) {                                   \
		/* Init on first TEST(). */                          \
		if (_wh.all == 0) _wh.fname = __FILE__;              \
		if (_type == _WH_ONLY) _wh.flag |= _WH_O;            \
		_wh.its[_wh.all] = &__wh_test_body##id;              \
		_wh.msg[_wh.all] = _msg;                             \
		_wh.line[_wh.all] = _line;                           \
		_wh.type[_wh.all] = _type;                           \
		_wh.all++;                                           \
		if (_wh.all <= WH_MAX) return;                       \
		fprintf(stderr, "ERR exceeded WH_MAX (see doc)\n");  \
		exit(1);        /* Too many tests */                 \
	}                                                            \
	void __wh_test_body##id(void)

/* Intermediate macro function _WH_BASE is necessary to "unwrap"
 * __LINE__ macro so it could be used as a ID string. */
#define _WH_BASE(msg, id, type) __WH_BASE(msg, id, __LINE__, type)

/* Main test macros for defining test blocks. */
#define TEST(msg) _WH_BASE(msg, __LINE__, _WH_TEST)
#define SKIP(msg) _WH_BASE(msg, __LINE__, _WH_SKIP)
#define ONLY(msg) _WH_BASE(msg, __LINE__, _WH_ONLY)

/* Core assertion macro that every other assertion macro use. */
#define __WH_ASSERT(bool, onfail, line) do {                         \
		_wh.last_all++;                                      \
		if (bool) break;                /* Pass */           \
		_wh.last_err++;                 /* Fail */           \
		fprintf(stderr, "%s:%d:\t", _wh.fname, line);        \
		onfail;                                              \
		fputc('\n', stderr);                                 \
		if (_wh.flag & _WH_Q) return;   /* End quick */      \
	} while(0)

/* Helper assertion macros. */
#define _WH_ASSERT(bool, onfail) __WH_ASSERT(bool, onfail, __LINE__)
#define _WH_STR_EQ(a,b) ((a && b && !strcmp(a?a:"", b?b:"")) || (!a && !b))
#define _WH_BUF_EQ(a,b,n) (strncmp(a,b,n) == 0)

/* Basic assertions. */
#define ASSERT(x,msg) _WH_ASSERT(x, fputs(msg, stderr))
#define OK(x) ASSERT(x, "OK("#x")")
#define EQ(a,b) ASSERT((a) == (b), "EQ("#a", "#b")")
#define STR_EQ(a,b) _WH_ASSERT(_WH_STR_EQ(a,b),				\
			       fprintf(stderr, "STR_EQ(%s, %s)\n"	\
				       "\t'%s'\n\t'%s'",		\
				       #a, #b,				\
				       a?a:"<NULL>", b?b:"<NULL>"))
#define BUF_EQ(a,b,n) _WH_ASSERT(_WH_BUF_EQ(a,b,n),			\
				 fprintf(stderr, "BUF_EQ(%s, %s, %s)\n"	\
					 "\t'%.*s'\n\t'%.*s'",		\
					 #a, #b, #n,			\
					 (int)n, a, (int)n, b))
#define STR_NEQ(a,b)   ASSERT(!_WH_STR_EQ(a,b),   "STR_NEQ("#a", "#b")")
#define BUF_NEQ(a,b,n) ASSERT(!_WH_BUF_EQ(a,b,n), "BUF_NEQ("#a", "#b", "#n")")

/* Force end of test block. */
#define END() do {return;} while(0)

/* Run CMD assertions. */
#define RUN(cmd, in, out, err, code)				\
	ASSERT(_wh_run(cmd, in, out, err, code),		\
	       "RUN("#cmd", "#in", "#out", "#err", "#code")")
#define STR_RUN(cmd, in, out, err, code)			\
	ASSERT(_wh_str_run(cmd, in, out, err, code),		\
	       "STR_RUN("#cmd", "#in", "#out", "#err", "#code")")

enum {				/* Flags */
	_WH_V = 1,		/* Verbose mode */
	_WH_Q = 2,		/* Quick mode */
	_WH_F = 4,		/* Fast mode */
	_WH_O = 8		/* ONLY test macro was used */
};
enum {				/* Test macro types */
	_WH_TEST = 0,		/* Regular test */
	_WH_SKIP,		/* Skip test */
	_WH_ONLY,		/* Run only ONLY tests */
	__WH_SIZ		/* For _wh_type array size */
};
struct {			/* Tests global state */
	char  *fname;           /* Test source file name */
	int    flag;            /* Program flags, enum _WH_V... */
	int    all;             /* Number of all tests */
	int    err;             /* Number of failed tests */
	int    last_all;        /* Last test assertions count */
	int    last_err;        /* Last test assertions fails */
	int    line[WH_MAX];    /* TEST macros line in file */
	int    type[WH_MAX];    /* Test type enum WH_SKIP... */
	char  *msg[WH_MAX];     /* TEST macro messages */
	void (*its[WH_MAX])();	/* Test functions pointers */
} _wh = {0, 0, 0, 0, 0, 0, {0}, {0}, {0}, {0}};

/* String representations of test macro types. */
const char *_wh_type[__WH_SIZ] = {"TEST", "SKIP", "ONLY"};

/* Compare A and B buffers content of SIZ size.  Return index to
 * first byte that is different or -1 when buffers are the same. */
int _wh_cmp(char *a, char *b, ssize_t siz);

/* Compare files pointed by FD0 and FD1 file desciptors.  Print error
 * message showing where the differance is if content is not the
 * same.  Return 0 when files are the same. */
int _wh_fdcmp(int fd0, int fd1);

/* Split STR string with SPLIT char.  Return array of null terminated
 * strings with NULL being last array element.  Original STR string
 * will not be modified. */
char **_wh_split(char *str, char split);

/* Test CMD.  IN, OUT and ERR are optional paths to files used as
 * stdin, stdou and stderr, can be ommited by setting them to NULL.
 * Function will run CMD command with IN file content if given and
 * test if stdout is equal to content of OUT file if given, same for
 * ERR and will compare CODE expected program exit code with actuall
 * CMD exit code.  Return 0 on failure. */
int _wh_run(char *cmd, char *in, char *out, char *err, int code);

/* Works the same as _wh_run but IN, OUT and ERR are not paths to
 * files but strings. */
int _wh_str_run(char *cmd, char *in, char *out, char *err, int code);

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
		/* TODO(irek): Printing extra fail data, like in case
		 * of str_eq should happen before printing line with
		 * error message pointing to file and assertion line.
		 * This is because some assertions might be able to
		 * print error only during runtime.  For example while
		 * reading file.  Later when you know that assertion
		 * failed you don't rly know where an why.  You know
		 * it at the moment when it fails so it's much better
		 * to print error right away. */
		(*_wh.its[i])(); /* Run test, print assert fails */
		if (_wh.last_err) {
			_wh.err++;
			fprintf(stderr, "%s:%d:\t%s %s\n",
				_wh.fname,
				_wh.line[i],
				_wh_type[_wh.type[i]],
				_wh.msg[i]);
		}
		if (_wh.flag & _WH_V) {
			printf("%s %s\t(%d/%d) pass\n",
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
		printf("FILE %s\t(%d/%d) pass\n", _wh.fname,
		       _wh.all - _wh.err, _wh.all);
	} else if (_wh.err) {
		fprintf(stderr, "%s\t%d err\n", _wh.fname, _wh.err);
	}
	return _wh.err;
}

int
_wh_cmp(char *a, char *b, ssize_t siz)
{
	ssize_t i = 0;
	while (i < siz && a[i] == b[i]) i++;
	return i < siz ? i : -1;
}

int
_wh_fdcmp(int fd0, int fd1)
{
	int show;		/* How many bytes print on error */
	ssize_t diff=-1;	/* FD0 difference index, -1 no diff */
	ssize_t sum=0;		/* Sum of read bytes */
	ssize_t siz0, siz1;	/* Size of read buffer */
	char buf0[BUFSIZ];	/* Buffer for reading from fd0 */
	char buf1[BUFSIZ];	/* Buffer for reading from fd1 */
	while ((siz0 = read(fd0, buf0, BUFSIZ)) > 0) {
		/* We should be able to read the same amount of bytes
		 * SIZ0 from FD1 as we read from FD0.  So we should at
		 * leas be able to read from FD1 and SIZ0 and SIZ1
		 * should be the same. */
		if ((siz1 = read(fd1, buf1, siz0)) == -1) {
			/* TODO(irek): Instead of returning diff I
			 * should print error as soon as as it was
			 * found.  Mayber? */
			diff = 0;
			break;
		}
		/* Compare buffers.  Set DIFF to index of BUF0 when
		 * difference was found. */
		if ((diff = _wh_cmp(buf0, buf1, siz0)) >= 0) {
			break;
		}
		if (siz0 != siz1) {
			diff = siz0 < siz1 ? siz0 : siz1;
			break;
		}
		sum += siz0;
	}
	/* At this point BUF0 was read in it's entirely but BUF1 might
	 * still hold more data.  If difference was not found at this
	 * point then check if there is still something in BUF1 which
	 * means buffers are different at last position of BUF0. */
	if (diff == -1 && (siz1 = read(fd1, buf1, BUFSIZ)) > 0) {
		diff = siz0;
	}
	/* When diff is a valid BUFF0 index (not -1) then we found
	 * difference at that index.  If so then print WH_SHOW amount
	 * of bytes (if possible because there might be less bytes
	 * available) up to the DIFF index showing where is first
	 * difference along with index to that byte. */
	if (diff >= 0) {
		show = (diff > WH_SHOW ? WH_SHOW : diff);
		fprintf(stderr, "\t'%.*s' < difference at byte %ld\n",
			show, &buf0[diff-show+1], sum+diff);
		return 1;
	}
	return 0;
}

int
_wh_run(char *cmd, char *sin, char *sout, char *serr, int code)
{
	int fd;			/* sin, sout, serr file desciptor */
	int fd_in[2];		/* File desciptors for input pipe */
	int fd_out[2];		/* File desciptors for output pipe */
	int fd_err[2];		/* File desciptors for error pipe */
	int ws, wes;		/* Wait status, wait exit status */
	pid_t pid;		/* CMD process id */
	char buf[BUFSIZ];	/* For passing stdin to PID SIN pipe */
	ssize_t ssiz;		/* Signed size, to read bytes */
	/* Open pipes.  We need pipe for stdin, stdout and asderr. */
	if (pipe(fd_in)  == -1) err(1, "pipe(fd_in)");
	if (pipe(fd_out) == -1) err(1, "pipe(fd_out)");
	if (pipe(fd_err) == -1) err(1, "pipe(fd_err)");
	/* Fork process to run CMD as a child. */
	if ((pid = fork()) == -1) err(1, "fork");
	if (pid == 0) {		  /* Child process, the CMD */
		close(fd_in[1]);  /* Index 1 is for writing. */
		close(fd_out[0]); /* Index 0 is for reading */
		close(fd_err[0]);
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		dup(fd_in[0]);
		dup(fd_out[1]);
		dup(fd_err[1]);
		if (execl("/bin/sh", "sh", "-c", cmd, 0) == -1) {
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
		return 0;	/* Failed */
	}
	return 1;		/* Success */
}

int
_wh_str_run(char *cmd, char *sin, char *sout, char *serr, int code)
{
	FILE *fp;
	if (sin) {
		if (!(fp=fopen(WH_IN, "w"))) err(1, "fopen(sin)");
		if (fputs(sin, fp) == EOF)   err(1, "fputs(sin)");
		if (fclose(fp))              err(1, "fopen(sin)");
		sin = WH_IN;
	}
	if (sout) {
		if (!(fp=fopen(WH_OUT, "w"))) err(1, "fopen(sout)");
		if (fputs(sout, fp) == EOF)   err(1, "fputs(sout)");
		if (fclose(fp))               err(1, "fopen(sout)");
		sout = WH_OUT;
	}
	if (serr) {
		if (!(fp=fopen(WH_ERR, "w"))) err(1, "fopen(serr)");
		if (fputs(serr, fp) == EOF)   err(1, "fputs(serr)");
		if (fclose(fp))               err(1, "fopen(serr)");
		serr = WH_ERR;
	}
	return _wh_run(cmd, sin, sout, serr, code);
}
/*
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
