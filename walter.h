/* Walter is a single header library for writing unit tests in C made
 * with fewer complications.
 *
 * v2.0 from https://github.com/ir33k/walter by Irek
 *
 * Example test program:
 *
 *	// File: example.t.c
 *	#define WH_MAX 1024             // Define to handle more tests
 *	#include "walter.h"
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
 *	SKIP("TODO test 4") {}          // Can be used for TODOs
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
 *	$ ./example.t -q                # End quick on first fail
 *	$ ./example.t -vq               # Captain obvious
 *
 * Can be included only in one test program because it has single
 * global tests state, it defines it's own "main" function and TEST
 * macro relays on file line numbers.
 *
 * You can define only 64 tests by default but this can be changed by
 * predefined WH_MAX (see example).  Variables, functions and macros
 * not mentioned in example test program should not be used.
 */
#ifdef WALTER_H_
#error "walter.h library can't be used in multiple files (read doc)"
#else
#define WALTER_H_

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#ifndef WH_MAX			/* Predefined for more tests */
#define WH_MAX    64		/* Maximum number of tests */
#endif				/* BTW WH_ stands for Walter.H */

#define WH____TEST(_msg, id, _line, _type)				\
	void wh____test_body##id(void);					\
	/* This function will be run before "main" function. */		\
	void wh____test##id(void) __attribute__ ((constructor));	\
	void wh____test##id(void) {					\
		/* Init on first TEST(). */				\
		if (wh__.all == 0) wh__.fname = __FILE__;		\
		if (_type == WH_ONLY) wh__.only = 1;			\
		wh__.its[wh__.all] = &wh____test_body##id;		\
		wh__.msg[wh__.all] = _msg;				\
		wh__.line[wh__.all] = _line;				\
		wh__.type[wh__.all] = _type;				\
		wh__.all++;						\
		if (wh__.all > WH_MAX) {				\
			fprintf(stderr, "ERROR: WH_MAX is %d (read doc)\n", \
				WH_MAX);				\
			exit(1);					\
		}							\
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
		if (wh__.quick) return; /* Stop on first fail */	\
	} while(0)
#define WH__ASSERT(bool, onfail) WH____ASSERT(bool, onfail, __LINE__)
#define WH__STR_EQ(a,b) ((a && b && strcmp(a?a:"", b?b:"") == 0) || (!a && !b))
#define WH__BUF_EQ(a,b,n) (strncmp(a,b,n) == 0)

#define ASSERT(x,m) WH__ASSERT(x, fputs(m, stderr))
#define OK(x) ASSERT(x, "OK("#x")")
#define STR_EQ(a,b) WH__ASSERT(WH__STR_EQ(a,b),				\
			       fprintf(stderr,				\
				       "STR_EQ(%s, %s)\n"		\
				       "\t'%s'\n\t'%s'",		\
				       #a, #b,				\
				       a ? a : "<NULL>", b ? b : "<NULL>"))
#define BUF_EQ(a,b,n) WH__ASSERT(WH__BUF_EQ(a,b,n),			\
				 fprintf(stderr,			\
					 "BUF_EQ(%s, %s, %s)\n"		\
					 "\t'%.*s'\n\t'%.*s'",		\
					 #a, #b, #n,			\
					 (int)n, a, (int)n, b))
#define STR_NEQ(a,b) ASSERT(!WH__STR_EQ(a,b), "STR_NEQ("#a", "#b")")
#define BUF_NEQ(a,b,n) ASSERT(!WH__BUF_EQ(a,b,n), "BUF_NEQ("#a", "#b", "#n")")
#define END() do { return; } while(0)

/* Test type. */
enum { WH_TEST, WH_SKIP, WH_ONLY };

/* Tests global state. */
struct {
	int    all;                 /* Number of all tests */
	int    err;                 /* Number of failed tests */
	int    last_all;            /* Last test assertions count */
	int    last_err;            /* Last test assertions fails */
	int    line[WH_MAX];        /* TEST macros line in file */
	int    type[WH_MAX];        /* Test type (test, skip, only) */
	int    only;                /* True if ONLY() was used */
	int    verb;                /* True if verbose mode enabled */
	int    quick;               /* True if quick mode enabled */
	char  *fname;               /* Test source file name */
	char  *msg[WH_MAX];         /* TEST macro messages */
	void (*its[WH_MAX])(void);  /* Test functions pointers */
} wh__ = {0, 0, 0, 0, {0}, {0}, 0, 0, 0, NULL, {NULL}, {NULL}};

/* Runs all tests defined with TEST macro.  Program returns number of
 * failed tests or 0 on success. */
int
main(int argc, char **argv)
{
	int i, opt;

	while ((opt = getopt(argc, argv, "vqh")) == 0) {
		switch (opt) {
		case 'v': wh__.verb  = 1; break;
		case 'q': wh__.quick = 1; break;
		case 'h':
		default:
			printf("usage: %s [-vqh]\n\n"
			       "\t-v\tRun in verbose mode.\n"
			       "\t-q\tStop current test on first fail.\n"
			       "\t-h\tPrints this help usage message.\n",
			       argv[0]);
			return 1;
		}
	}
	/* Run tests.  Print error on fail. */
	for (i = 0; i < wh__.all; i++) {
		if (wh__.only && wh__.type[i] != WH_ONLY) {
			continue;
		}
		if (wh__.type[i] == WH_SKIP) {
			fprintf(stderr, "%s:%d: note:\tSKIP %s\n",
				wh__.fname, wh__.line[i], wh__.msg[i]);
			continue;
		}
		wh__.last_all = 0;
		wh__.last_err = 0;
		(*wh__.its[i])();   /* Run, print assertion fails */
		if (wh__.last_err) {
			wh__.err++;
			fprintf(stderr, "%s:%d: error:\%s %s\n",
				wh__.fname,
				wh__.line[i],
				wh__.only ? "ONLY" : "TEST",
				wh__.msg[i]);
		}
		if (wh__.verb) {
			printf("%s %s\t(%d/%d) pass\n",
			       wh__.only ? "ONLY" : "TEST",
			       wh__.msg[i],
			       wh__.last_all - wh__.last_err,
			       wh__.last_all);
		}
	}
	/* Print results summary. */
	if (wh__.verb) {
		printf("FILE %s\t(%d/%d) pass\n", wh__.fname,
		       wh__.all - wh__.err, wh__.all);
	} else {
		printf("%s\t%d err\n", wh__.fname, wh__.err);
	}
	return wh__.err;
}
#endif	/* WALTER_H_ */

/*
This software is available under 2 licenses, choose whichever.

----------------------------------------------------------------------
ALTERNATIVE A - MIT License, Copyright (c) 2017 Sean Barrett

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions: The above copyright notice and this
permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

----------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)

This is free and unencumbered software released into the public
domain.  Anyone is free to copy, modify, publish, use, compile, sell,
or distribute this software, either in source code form or as a
compiled binary, for any purpose, commercial or non-commercial, and by
any means.  In jurisdictions that recognize copyright laws, the author
or authors of this software dedicate any and all copyright interest in
the software to the public domain. We make this dedication for the
benefit of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
