/* walter.h v5.0 from https://github.com/ir33k/walter by irek@gabr.pl

Walter is a single header library for writing unit tests in C made
with fewer complications by avoiding boilerplate.

Table of contents:

	Example
	Usage
	Disclaimers
	Changelog
	Licenses (at the very end of this file)

Example:

	#include "walter.h"

	TEST("Test description")        // Define test with assertions
	{
		ASSERT(bool, "msg");    // Print message on false
		OK(bool);               // Is boolean true?
		EQ(b1, b2, size);       // Are buffers equal?
		NEQ(b1, b2, size);      // Are buffers not equal?
		EQ(s1, s2, -1);         // Are strings equal?
		NEQ(s1, s2, -1);        // Are strings not equal?
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

		// If you want to use string literals instead of file
		// paths then use STR macro as prefix.
		RUN("tr abc 123", STR"AaBbCc", STR"A1B2C3", 0, 0);
	}
	TEST("Another test 1") {...}    // Define as many as WH_MAX
	SKIP("Another test 2") {...}    // Skip or just ignore test
	SKIP("Another test 3") {}       // Body can be empty
	SKIP("TODO test 4")    {}       // Can be used for TODOs
	ONLY("Another test 5") {...}    // Ignore all other tests

	// There is no main() function

Usage:

	$ cc test.c             # Compile
	$ ./a.out -h            # Print help
	$ ./a.out               # Run tests
	$ echo $?               # Number of failed tests

Disclaimers:

	1. Library can be included only once because it has global
	   state, relays on file line numbers and defines main().
	2. It's expected that variables, functions and macros that are
	   not mentioned in example are not used in test programs.
	3. You can define up to 256 tests per file.  If you need more
	   then change WH_MAX value in your copy of walter.h.
	4. When buffer or string assertion fails Walter prints small
	   part of arguments as preview.  If you need different length
	   of that preview then modify WH_SHOW value.
	5. I encourage you to modify source code.  If some macro name
	   is in conflict to your existing macro then rename it.  If
	   you need custom assert macro, then add it.  Source code is
	   short and easy to change.
	6. WH_ prefix stands for Walter.H.  _WH_ is for private stuff.
	   __WH_ is for super epic internal private stuff, just move
	   along, this is not the code you are looking for \(-_- )

Changelog:

	2024.01.25	v5.0

	2024.01.09	v4.1

	1. Fix typos by Kian-Meng Ang <kianmeng@cpan.org>.
	2. Add -a flag for printing absolute file paths.
	3. Remove arrow when printing error message in _wh_eq.

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
	2. Extend documentation with table of contents and change log.

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

#define WH_MAX  256             /* Maximum number of tests */
#define WH_SHOW 32              /* How many chars print on error */
#define WH_TMP "/tmp/walter"    /* Path to tmp file for RUN() */
#define STR "\0"                /* 1 char prefix for RUN() args */

#define __WH_TEST(Desc, Id, Line)                                    \
	void __wh_body##Id();                                        \
	void __wh_head##Id() __attribute__ ((constructor));          \
	void __wh_head##Id() {                                       \
		assert(_wh_all < WH_MAX);                            \
		if (!_wh_file) _wh_file = __FILE__;                  \
		if (Desc[0] == 'O') _wh_only = 1;                    \
		_wh_desc[_wh_all] = Desc;                            \
		_wh_line[_wh_all] = Line;                            \
		_wh_func[_wh_all] = &__wh_body##Id;                  \
		_wh_all++;                                           \
	}                                                            \
	void __wh_body##Id()
#define _WH_TEST(desc, id) __WH_TEST(desc, id, __LINE__)

#define TEST(desc) _WH_TEST("TEST "desc, __LINE__)
#define SKIP(desc) _WH_TEST("SKIP "desc, __LINE__)
#define ONLY(desc) _WH_TEST("ONLY "desc, __LINE__)

#define _WH_ASSERT(bool, msg, line) do {                             \
		if ((bool)) break;              /* Pass */           \
		_wh_mistake++;                  /* Fail */           \
		printf("%s:%d:\t%s\n", _wh_file, line, msg);         \
		if (_wh_quick) return;          /* End quick */      \
	} while(0)

#define ASSERT(a,msg) _WH_ASSERT(a, msg, __LINE__)
#define OK(a) ASSERT((a), "OK("#a")")
#define  EQ(a,b,n) ASSERT(_wh_eq(1,a,b,(size_t)n),  "EQ("#a", "#b", "#n")")
#define NEQ(a,b,n) ASSERT(_wh_eq(0,a,b,(size_t)n), "NEQ("#a", "#b", "#n")")
#define RUN(cmd, in, out, err, code)                                 \
	ASSERT(_wh_run(cmd, in, out, err, code),                     \
	       "RUN("#cmd", "#in", "#out", "#err", "#code")")

char *_wh_help =
"usage: %s [options]\n"
"\n"
"options:\n"
"	-q	Quick, stop TEST on first failed assertion.\n"
"	-l N	Limit, stop after N number of failed tests.\n"
"	-h	Prints this help message.\n";

char *_wh_file=0;               /* Path to test file */
int _wh_quick=0;                /* True for -q option */
int _wh_all=0;                  /* Number of all tests */
int _wh_only=0;                 /* Non 0 when ONLY() macro was used */
int _wh_mistake;                /* Number of failed assertions in test */
char *_wh_desc[WH_MAX];         /* TEST() type + description */
int _wh_line[WH_MAX];           /* TEST() line number in file */
void (*_wh_func[WH_MAX])();     /* TEST() functions pointers */

/* Compare buffer A with buffer B of SZ size.  When SZ is -1 then it's
 * assumend that A and B are null terminated strings.  Return non 0
 * value when EQ value is 1 and buffers are the same, or when EQ value
 * is 0 and buffers are different. */
int _wh_eq(int eq, char *a, char *b, size_t sz);

/* Compare files pointed by A and B file descriptors.  Print error
 * message showing where the difference is if content is not the same.
 * Return 0 when files are the same. */
int _wh_fdcmp(int a, int b);

/* Test CMD.  IN, OUT and ERR are optional paths to files used as
 * stdin, stdou and stderr, can be omitted by setting them to NULL.
 * Function will run CMD command with IN file content if given and
 * test if stdout is equal to content of OUT file if given, same for
 * ERR and will compare CODE expected program exit code with actually
 * CMD exit code.  Return 0 on failure. */
int _wh_run(char *cmd, char *In, char *Out, char *Err, int code);

/* Runs _WH_TEST macros, return number of failed tests. */
int
main(int argc, char **argv)
{
	int i=1, fail=0, limit=WH_MAX;
	while ((i = getopt(argc, argv, "ql:h")) != -1) switch (i) {
		case 'q': _wh_quick = 1; break;
		case 'l': limit = atoi(optarg); break;
		default: printf(_wh_help, argv[0]); return 1;
	};
	for (i=0; i < _wh_all && fail < limit; i++) {
		if (_wh_only && _wh_desc[i][0] != 'O')
			continue;
		_wh_mistake = 0;
		if (_wh_desc[i][0] != 'S')
			(*_wh_func[i])();
		if (_wh_mistake)
			fail++;
		if (_wh_mistake || _wh_desc[i][0] == 'S')
			printf("%s:%d:\t%s\n",
			       _wh_file, _wh_line[i], _wh_desc[i]);
	}
	if (fail)
		printf("%s\t%d fail\n", _wh_file, fail);
	return fail;
}

int
_wh_eq(int eq, char *a, char *b, size_t sz)
{
	size_t i=0, n=sz, m=sz, offset;
	if (sz == (size_t)-1) {
		n = a ? strlen(a) : 0;
		m = b ? strlen(b) : 0;
	}
	if (!a || !b) {         /* Handle nulls */
		if (eq == (a == b))
			return 1;
	} else {
		while (i<n && i<m && a[i] == b[i])
			i++;
		if (eq == (a[i-1] == b[i-1] && n == i && m == i))
			return 1;
	}
	offset = i - (i % WH_SHOW);
	a += offset; n -= offset;
	b += offset; m -= offset;
	if (n > WH_SHOW) n = WH_SHOW;
	if (m > WH_SHOW) m = WH_SHOW;
	printf("\t First incorrect byte: %lu\n"
	       "\t\"%.*s\"\n"
	       "\t\"%.*s\"\n",
	       i,
	       (int)n, a ? a : "<NULL>",
	       (int)m, b ? b : "<NULL>");
	return 0;
}

int
_wh_fdcmp(int a, int b)
{
	ssize_t n;
	char buf_a[BUFSIZ];
	char buf_b[BUFSIZ];
	while ((n = read(a, buf_a, sizeof buf_a)) > 0) {
		if (read(b, buf_b, n) != n)
			return 1;
		if (!_wh_eq(1, buf_a, buf_b, n))
			return 1;
	}
	/* At this point A was read in it's entirely but B might still
	 * hold more data.  If difference was not found at this point
	 * then check if there is still something in B which means
	 * buffers are different at last position of A.  When there is
	 * nothing to read then we have success. */
	return read(b, buf_b, sizeof buf_b) > 0;
}

int
_wh_run(char *cmd, char *In, char *Out, char *Err, int code)
{
	int fd, fd0[2], fd1[2], fd2[2];
	int cmp, ws, wes;
	pid_t pid;
	char buf[BUFSIZ];
	ssize_t n;
	assert(cmd);
	if (pipe(fd0) == -1) err(1, "pipe(in)");
	if (pipe(fd1) == -1) err(1, "pipe(out)");
	if (pipe(fd2) == -1) err(1, "pipe(err)");
	if ((pid = fork()) == -1) err(1, "fork");
	if (pid == 0) {           /* Child process, the CMD */
		/* Redirect std in, out and err of child to my pipe
		 * files.  Index 1 is for writing, 0 for reading */
		close(fd0[1]); close(0); dup(fd0[0]);
		close(fd1[0]); close(1); dup(fd1[1]);
		close(fd2[0]); close(2); dup(fd2[1]);
		if (execl("/bin/sh", "sh", "-c", cmd, NULL) == -1) {
			/* TODO(irek): Probably a bug, how it supposed
			 * to report the error when being the child
			 * process and the return 0 should probably be
			 * replaced with exit(1). */
			perror("execl");
			return 0;
		}
		close(fd0[0]);
		close(fd1[1]);
		close(fd2[1]);
		exit(0);
	}
	/* Parent process */
	close(fd0[0]);
	close(fd1[1]);
	close(fd2[1]);
	/* Pass standard input */
	if (In) {
		if (In[0] == STR[0]) {
			if (write(fd0[1], In+1, strlen(In+1)) == -1) {
				perror("write(In)");
				return 0;
			} 
		} else {
			if ((fd = open(In, O_RDONLY)) == -1)
				err(1, "open(%s)", In);
			while ((n = read(fd, buf, BUFSIZ)) > 0) {
				if (write(fd0[1], buf, n) == -1) {
					perror("write(In)");
					return 0;
				}
			}
			if (close(fd) == -1)
				err(1, "close(%s)", In);
		}
	}
	if (close(fd0[1]) == -1)
		err(1, "close(fd0[1])");
	/* Compare standard output */
	if (Out) {
		if (Out[0] == STR[0]) {
			if ((fd = open(WH_TMP, O_RDWR | O_CREAT | O_TRUNC, 0600)) == -1)
				err(1, "open(tmp/Out)");
			if (write(fd, Out+1, strlen(Out+1)) == -1)
				err(1, "write(tmp/Out)");
			if (close(fd) == -1)
				err(1, "close(tmp/Out)");
			Out = WH_TMP;
		}
		if ((fd = open(Out, O_RDONLY)) == -1)
			err(1, "open(%s)", Out);
		cmp = _wh_fdcmp(fd1[0], fd);
		if (close(fd) == -1)
			err(1, "close(%s)", Out);
		if (cmp) {
			printf("\tstdout\n");
			return 0;
		}
	}
	if (close(fd1[0]) == -1)
		err(1, "close(fd1[0])");
	/* Compare standard error */
	if (Err) {
		if (Err[0] == STR[0]) {
			if ((fd = open(WH_TMP, O_RDWR | O_CREAT | O_TRUNC, 0600)) == -1)
				err(1, "open(tmp/Err)");
			if (write(fd, Err+1, strlen(Err+1)) == -1)
				err(1, "write(tmp/Err)");
			if (close(fd) == -1)
				err(1, "close(tmp/Err)");
			Err = WH_TMP;
		}
		if ((fd = open(Err, O_RDONLY)) == -1)
			err(1, "open(%s)", Err);
		cmp = _wh_fdcmp(fd2[0], fd);
		if (close(fd) == -1)
			err(1, "close(%s)", Err);
		if (cmp) {
			printf("\tstderr\n");
			return 0;
		}
	}
	if (close(fd2[0]) == -1)
		err(1, "close(fd2[0])");
	/* Wait for child process to exit */
	if (waitpid(pid, &ws, 1) == -1) {
		perror("waitpid");
		return 0;
	}
	/* Get child process exit code */
	if (WIFEXITED(ws) && (wes = WEXITSTATUS(ws)) != code) {
		printf("\tExpected exit code %d, got %d\n",
			code, wes);
		return 0;
	}
	return 1;
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
