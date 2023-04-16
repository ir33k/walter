/* David is an extension to walter.h providing more assertion macros.
 * Mostly for working with files and running processes.
 *
 * david.h v0.4 from https://github.com/ir33k/walter by Irek
 *
 * Example usage:
 *
 *	// File: example.t.c
 *	#include "walter.h"             // Include main lib
 *	#include "david.h"              // Include extension
 *
 *	TEST("Test stdin, stdout, stderr and exit code of grep")
 *	{
 *		IOE("grep -ir abc",     // CMD command to test
 *		    "stdin.txt",        // STDIN passed to cmd
 *		    "stdout.txt",       // Expected stdout
 *		    "stderr.txt",       // Expected stderr
 *		    0);                 // Expected exit code
 *	}
 */
#ifndef WALTER_H_
#	error "david.h requires walter.h to be included first"
#endif
#ifdef DAVID_H_
#	error "david.h can't be included multiple times (read doc)"
#endif
#define DAVID_H_

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef DH_SHOW			/* Predefined to show more bytes */
#define DH_SHOW 32		/* Number of bytes to show on fail */
#endif				/* BTW DH_ stands for David.H */

#define IOE(cmd, in, out, err, code)			\
	ASSERT(dh__ioe(cmd, in, out, err, code),	\
	       "IOE("#cmd", "#in", "#out", "#err", "#code")")

/* Split STR string with SPLIT char.  Return array of null terminated
 * strings with NULL being last array element.  All array strings are
 * stored in single memory buffer so first array element points at
 * whole buffer.  This should be used to free alocated memory. */
char **dh__malloc_split(char *str, char split);

/* Compare BUF0 and BUF1 content of SIZ size.  Return index to first
 * byte that is different or -1 when buffers are the same. */
int dh__bufncmp(char *buf0, char *buf1, ssize_t siz);

/* Compare files pointed by FD0 and FD1 file desciptors.  Print error
 * message showing where the differance is if content is not the
 * same.  Return 0 if files are not the same. */
int dh__fdcmp(int fd0, int fd1);

/* IOE stands for Input, Output, Error.  IN, OUT and ERR are optional
 * path to files used as stdin, stdou and stderr, can be ommited by
 * setting them to NULL.  Function will run CMD command with IN file
 * content if given and test if stdout is equal to content of OUT file
 * if given, same for ERR and will compare CODE expected program exit
 * code with actuall CMD exit code.  Return 0 on failure. */
int dh__ioe(char *cmd, char *in, char *out, char *err, int code);

int
dh__bufncmp(char *buf0, char *buf1, ssize_t siz)
{
	ssize_t i;
	for (i = 0; i < siz; i++) {
		if (buf0[i] != buf1[i]) {
			return i; /* Nope, buffers are not equal */
		}
	}
	return -1;		/* Buffers are equal */
}

int
dh__fdcmp(int fd0, int fd1)
{
	int show;		/* How many char print on error */
	ssize_t diff=-1;	/* FD0 difference index, -1 no diff */
	ssize_t sum=0;		/* Sum of read bytes */
	ssize_t siz0, siz1;	/* Size of read buffer */
	char buf0[BUFSIZ];	/* Buffer for reading from fd0 */
	char buf1[BUFSIZ];	/* Buffer for reading from fd1 */
	while ((siz0 = read(fd0, buf0, BUFSIZ)) > 0) {
		if ((siz1 = read(fd1, buf1, siz0)) == -1) {
			diff = 0;
			break;
		}
		if (siz0 != siz1) {
			diff = siz0 < siz1 ? siz0 : siz1;
			break;
		}
		if ((diff = dh__bufncmp(buf0, buf1, siz0)) >= 0) {
			break;
		}
		sum += siz0;
	}
	if (diff == -1 && (siz1 = read(fd1, buf1, BUFSIZ)) > 0) {
		diff = siz0;
	}
	if (diff >= 0) {
		show = (diff > DH_SHOW ? DH_SHOW : diff);
		fprintf(stderr, "\t'%.*s' < difference at byte %ld\n",
			show, &buf0[diff-show+1], sum+diff);
		return 0;
	}
	return 1;
}

char **
dh__malloc_split(char *str, char split)
{
	char *buf, **res;
	size_t i, j=1, siz;
	siz = strlen(str);
	/* Main memory buffer is essentially a given STR but SPLIT
	 * characters will be replaced with 0. */
	buf = malloc(siz);
	memcpy(buf, str, siz);
	for (i=0; i < siz; i++) {
		if (buf[i] == split) {
			buf[i] = 0;
			j++;
		}
	}
	res = malloc(sizeof(char*) * (j+1)); /* +1 for null terminator */
	res[0] = buf;
	for (j=1, i=0; i < siz; i++) {
		if (buf[i] == 0) {
			res[j++] = &buf[i+1];
		}
	}
	res[j] = NULL;
	return res;
}

int
dh__ioe(char *cmd, char *in, char *out, char *err, int code)
{
	int fd;			/* File desciptor for in, out, err */
	int fd_in[2];		/* File desciptors for input pipe */
	int fd_out[2];		/* File desciptors for output pipe */
	int fd_err[2];		/* File desciptors for error pipe */
	int ws, wes;		/* Wait status, wait exit status */
	pid_t pid;		/* CMD process id */
	char buf[BUFSIZ];	/* For passing stdin to PID IN pipe */
	ssize_t ssiz;		/* Signed size, to read bytes */
	char **args;		/* CMD program args */
	args = dh__malloc_split(cmd, ' ');
	/* Open pipes */
	if (pipe(fd_in)  == -1) { perror("pipe(fd_in)");  return 1; }
	if (pipe(fd_out) == -1) { perror("pipe(fd_out)"); return 1; }
	if (pipe(fd_err) == -1) { perror("pipe(fd_err)"); return 1; }
	/* Fork process to run CMD */
	if ((pid = fork()) == -1) {
		perror("fork");
		return 0;
	}
	if (pid == 0) {		/* Child process */
		/* Use pipe file descriptors instead of stadard fds. */
		close(fd_in[1]);  close(STDIN_FILENO);  dup(fd_in[0]);
		close(fd_out[0]); close(STDOUT_FILENO); dup(fd_out[1]);
		close(fd_err[0]); close(STDERR_FILENO); dup(fd_err[1]);
		if (execv(args[0], args) == -1) { /* Run CMD */
			perror("execv");
			exit(1);
		}
		free(args[0]);
		close(fd_in[0]);
		close(fd_out[1]);
		close(fd_err[1]);
		exit(0);	/* End child process */
	}
	/* Parent process */
	close(fd_in[0]);
	close(fd_out[1]);
	close(fd_err[1]);
	if (in) {		/* Pass stdin if defined to child */
		if ((fd = open(in, O_RDONLY)) == -1) {
			perror("open(in)");
			return 0;
		}
		while ((ssiz = read(fd, buf, BUFSIZ)) > 0) {
			if (write(fd_in[1], buf, ssiz) == -1) {
				perror("write(in)");
				return 0;
			}
		}
		if (close(fd) == -1) {
			perror("close(in)");
			return 0;
		}
	}
	close(fd_in[1]);
	if (out) {		/* Compare stdout if defined */
		if ((fd = open(out, O_RDONLY)) == -1) {
			perror("open(out)");
			return 0;
		}
		if (dh__fdcmp(fd_out[0], fd) == 0) {
			return 0;
		}
		if (close(fd) == -1) {
			perror("close(out)");
			return 0;
		}
	}
	close(fd_out[0]);
	if (err) {		/* Compare stderr if defined */
		if ((fd = open(err, O_RDONLY)) == -1) {
			perror("open(err)");
			return 0;
		}
		if (dh__fdcmp(fd_err[0], fd) == 0) {
			return 0;
		}
		if (close(fd) == -1) {
			perror("close(err)");
			return 0;
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
		fprintf(stderr, "Error code (expected, received): %d, %d\n",
			code, wes);
		return 0;
	}
	return 1;
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
