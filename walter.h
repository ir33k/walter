/* Walter is a single header library for writing unit tests in C made
 * with fewer complications.
 *
 * walter.h v2.1 from https://github.com/ir33k/walter by Irek
 *
 * Example usage:
 *
 *	// File: example.t.c
 *	#define WH_MAX 1024             // Optional, number of tests
 *	#include "walter.h"             // Include test lib
 *
 *	TEST("Test message")            // Define test with assertions
 *	{
 *		OK(bool);               // Is boolean true?
 *		ASSERT(bool, "text");   // OK() with custom message
 *		STR_EQ(s1, s2);         // Are strings equal?
 *		BUF_EQ(b1, b2, size);   // Are buffers equal?
 *		END();                  // End test here
 *		STR_NEQ(s1, s2);        // Are strings not equal?
 *		BUF_NEQ(b1, b2, size);  // Are buffers not equal?
 *	}
 *	TEST("Another test 1") { ... }  // Define as many as WH_MAX
 *	SKIP("Another test 2") { ... }  // Skip, ignore test
 *	SKIP("Another test 3") {}       // Body can be empty
 *	SKIP("TODO test 4")    {}       // Can be used for TODOs
 *	ONLY("Another test 5") { ... }  // Ignore all other tests
 *
 *	// There is no "main()" function
 *
 * Compile and run:
 *
 *	$ cc -o example.t example.t.c   # Compile
 *	$ ./example.t -h                # Print usage help
 *	$ ./example.t                   # Run tests
 *	$ ./example.t -v                # Run in verbose mode
 *	$ ./example.t -q                # Quick
 *	$ ./example.t -vq               # Combain
 *
 * Can be included only in one test program because it has single
 * global tests state, it defines it's own "main" function and TEST
 * macro relays on file line numbers.
 *
 * By default You can define only 64 tests but this can be changed by
 * predefining WH_MAX (see example).  Variables, functions and macros
 * not mentioned in example test program should not be used.
 */
#ifdef WALTER_H_
#	error "walter.h can't be included multiple times (read doc)"
#endif
#define WALTER_H_

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#ifndef WH_MAX			/* Predefined to handle more tests */
#define WH_MAX  64		/* Maximum number of test macros */
#endif				/* BTW WH_ stands for Walter.H */

#define WH____TEST(_msg, id, _line, _type)				\
	void wh____test_body##id(void);					\
	/* This function will be run before "main" function. */		\
	void wh____test##id(void) __attribute__ ((constructor));	\
	void wh____test##id(void) {					\
		/* Init on first TEST(). */				\
		if (wh__.all == 0) wh__.fname = __FILE__;		\
		if (_type == WH_ONLY) wh__.flag |= WH_O;		\
		wh__.its[wh__.all] = &wh____test_body##id;		\
		wh__.msg[wh__.all] = _msg;				\
		wh__.line[wh__.all] = _line;				\
		wh__.type[wh__.all] = _type;				\
		wh__.all++;						\
		if (wh__.all <= WH_MAX) return;				\
		fprintf(stderr, "ERR exceeded WH_MAX (see doc)\n");	\
		exit(1);        /* Too many tests */			\
	}								\
	void wh____test_body##id(void)
/* Intermediate macro function WH__TEST is necessary to "unwrap"
 * __LINE__ macro so it could be used as a ID string. */
#define WH__TEST(msg, id, type) WH____TEST(msg, id, __LINE__, type)
#define TEST(msg) WH__TEST(msg, __LINE__, WH_TEST)
#define SKIP(msg) WH__TEST(msg, __LINE__, WH_SKIP)
#define ONLY(msg) WH__TEST(msg, __LINE__, WH_ONLY)

#define WH____ASSERT(bool, onfail, line) do {				\
		wh__.last_all++;					\
		if (bool) break;	   /* Pass */			\
		wh__.last_err++;	   /* Fail */			\
		fprintf(stderr, "%s:%d: warning:\t", wh__.fname, line); \
		onfail;							\
		fputc('\n', stderr);					\
		if (wh__.flag & WH_Q) return; /* Stop on first fail */	\
	} while(0)
#define WH__ASSERT(bool, onfail) WH____ASSERT(bool, onfail, __LINE__)
#define WH__STR_EQ(a,b) ((a && b && !strcmp(a?a:"", b?b:"")) || (!a && !b))
#define WH__BUF_EQ(a,b,n) (strncmp(a,b,n) == 0)

#define ASSERT(x,m) WH__ASSERT(x, fputs(m, stderr))
#define OK(x) ASSERT(x, "OK("#x")")
#define STR_EQ(a,b) WH__ASSERT(WH__STR_EQ(a,b),				\
			       fprintf(stderr, "STR_EQ(%s, %s)\n"	\
				       "\t'%s'\n\t'%s'",		\
				       #a, #b,				\
				       a?a:"<NULL>", b?b:"<NULL>"))
#define BUF_EQ(a,b,n) WH__ASSERT(WH__BUF_EQ(a,b,n),			\
				 fprintf(stderr, "BUF_EQ(%s, %s, %s)\n"	\
					 "\t'%.*s'\n\t'%.*s'",		\
					 #a, #b, #n,			\
					 (int)n, a, (int)n, b))
#define STR_NEQ(a,b) ASSERT(!WH__STR_EQ(a,b), "STR_NEQ("#a", "#b")")
#define BUF_NEQ(a,b,n) ASSERT(!WH__BUF_EQ(a,b,n), "BUF_NEQ("#a", "#b", "#n")")
#define END() do {return;} while(0)

enum {				/* Flags */
	WH_NUL = 0,		/* No flags */
	WH_V   = 1 << 0,	/* Verbose mode */
	WH_Q   = 1 << 1,	/* Quick mode */
	WH_F   = 1 << 2,	/* Fast mode */
	WH_O   = 1 << 3		/* OMIT macro was used */
};
enum {WH_TEST, WH_SKIP, WH_ONLY, WH__SIZ}; /* Test types, arr siz */
struct {			    /* Tests global state */
	int    flag;                /* Program flags, enum WH_V... */
	int    all;                 /* Number of all tests */
	int    err;                 /* Number of failed tests */
	int    last_all;            /* Last test assertions count */
	int    last_err;            /* Last test assertions fails */
	int    line[WH_MAX];        /* TEST macros line in file */
	int    type[WH_MAX];        /* Test type enum WH_SKIP... */
	char  *fname;               /* Test source file name */
	char  *msg[WH_MAX];         /* TEST macro messages */
	void (*its[WH_MAX])(void);  /* Test functions pointers */
} wh__ = {0, 0, 0, 0, 0, {0}, {0}, NULL, {NULL}, {NULL}};
const char *wh__type[WH__SIZ] = {"TEST", "SKIP", "ONLY"};

/* Runs tests defined with TEST, SKIP and ONLY macros.
 * Return number of failed tests or 0 on success. */
int
main(int argc, char **argv)
{
	int i;
	while ((i = getopt(argc, argv, "vqfh")) != -1) {
		switch (i) {
		case 'v': wh__.flag |= WH_V; break;
		case 'q': wh__.flag |= WH_Q; break;
		case 'f': wh__.flag |= WH_F; break;
		case 'h':
		default:
			fprintf(stderr, "usage: %s [-vqh]\n\n"
			       "\t-v\tPrint verbose output.\n"
			       "\t-q\tQuick, show first fail in test.\n"
			       "\t-f\tFast, exit on first failed test.\n"
			       "\t-h\tPrints this help usage message.\n",
			       argv[0]);
			return 1;
		}
	}
	/* Run tests.  Print error on fail. */
	for (i = 0; i < wh__.all; i++) {
		if ((wh__.flag & WH_O) && wh__.type[i] != WH_ONLY) {
			continue;
		}
		if (wh__.type[i] == WH_SKIP) {
			fprintf(stderr, "%s:%d: note:\tSKIP %s\n",
				wh__.fname, wh__.line[i], wh__.msg[i]);
			continue;
		}
		wh__.last_all = 0;
		wh__.last_err = 0;
		/* TODO(irek): Printing extra fail data, like in case
		 * of str_eq should happen before printing line with
		 * error message pointing to file and assertion line.
		 * This is because some assertions might be able to
		 * print error only during runtime.  For example while
		 * reading file.  Later when you know that assertion
		 * failed you don't rly know where an why.  You know
		 * it at the moment when it fails so it's much better
		 * to print error right away. */
		(*wh__.its[i])(); /* Run test, print assert fails */
		if (wh__.last_err) {
			wh__.err++;
			fprintf(stderr, "%s:%d: error:\t%s %s\n",
				wh__.fname,
				wh__.line[i],
				wh__type[wh__.type[i]],
				wh__.msg[i]);
		}
		if (wh__.flag & WH_V) {
			printf("%s %s\t(%d/%d) pass\n",
			       wh__type[wh__.type[i]],
			       wh__.msg[i],
			       wh__.last_all - wh__.last_err,
			       wh__.last_all);
		}
		if (wh__.flag & WH_F && wh__.last_err) {
			break;
		}
	}
	/* Print verbose summary or errors if occured. */
	if (wh__.flag & WH_V) {
		printf("FILE %s\t(%d/%d) pass\n", wh__.fname,
		       wh__.all - wh__.err, wh__.all);
	} else if (wh__.err) {
		fprintf(stderr, "%s\t%d err\n", wh__.fname, wh__.err);
	}
	return wh__.err;
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
