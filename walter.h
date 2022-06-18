/* This is free and unencumbered software released into the public domain.
 * 
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 * 
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 * 
 * For more information, please refer to <http://unlicense.org/>
 */

/* Walter is a single header library for writing unit tests in C made
 * with fewer complications.
 *
 * v1.5 from https://github.com/ir33k/walter by Irek
 *
 * Example test program:
 *
 *	// File: example.t.c
 *	#define TESTMAX 1024            // Define to handle more tests
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
 *
 *	TEST("Another test 1") { ... }  // Define as many as TESTMAX
 *	TEST("Another test 2") { ... }
 *
 *	TODO("Another test 3") { ... }  // Mark test as TODO
 *	TODO("Another test 4") {}       // Body can be empty
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
 * predefined TESTMAX (see example).  Variables, functions and macros
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

#ifndef TESTMAX			/* Predefined for more tests */
#define TESTMAX    64		/* Maximum number of tests */
#endif

#define ____TEST(_msg, id, _line, _todo)				\
	void ____test_body##id(void);					\
	/* This function will be run before "main" function. */		\
	void ____test##id(void) __attribute__ ((constructor));		\
	void ____test##id(void) {					\
		/* Init on first TEST(). */				\
		if (__test.all == 0) __test.fname = __FILE__;		\
		__test.its[__test.all] = &____test_body##id;		\
		__test.msg[__test.all] = _msg;				\
		__test.line[__test.all] = _line;			\
		__test.todo[__test.all] = _todo;			\
		__test.all++;						\
		if (__test.all > TESTMAX) {				\
			fprintf(stderr, "ERROR: TESTMAX is %d (read doc)\n", \
				TESTMAX);				\
			exit(1);					\
		}							\
	}								\
	void ____test_body##id(void)
/* Intermediate macro function __TEST is necessary to "unwrap"
 * __LINE__ macro so it could be used as a ID string. */
#define __TEST(msg, id, todo) ____TEST(msg, id, __LINE__, todo)
#define TEST(msg) __TEST(msg, __LINE__, 0)
#define TODO(msg) __TEST(msg, __LINE__, 1)

#define ____ASSERT(bool, onfail, line) do {				\
		__test.last_all++;					\
		if (bool) break;                /* Pass */		\
		__test.last_err++;              /* Fail */		\
		fprintf(stderr, "%s:%d: warning:\t", __test.fname, line); \
		onfail;							\
		fputc('\n', stderr);					\
		if (__test.quick) return; /* Stop on first fail */	\
	} while(0)
#define __ASSERT(bool, onfail) ____ASSERT(bool, onfail, __LINE__)
#define __STR_EQ(a,b) ((a && b && strcmp(a?a:"", b?b:"") == 0) || (!a && !b))
#define __BUF_EQ(a,b,n) (strncmp(a,b,n) == 0)

#define ASSERT(x,m) __ASSERT(x, fputs(m, stderr))
#define OK(x) ASSERT(x, "OK("#x")")
#define STR_EQ(a,b) __ASSERT(__STR_EQ(a,b),				\
			     fprintf(stderr,				\
				     "STR_EQ(%s, %s)\n"			\
				     "\t'%s'\n\t'%s'",			\
				     #a, #b,				\
				     a ? a : "<NULL>", b ? b : "<NULL>"))
#define BUF_EQ(a,b,n) __ASSERT(__BUF_EQ(a,b,n),				\
			       fprintf(stderr,				\
				       "BUF_EQ(%s, %s, %s)\n"		\
				       "\t'%.*s'\n\t'%.*s'",		\
				       #a, #b, #n,			\
				       (int)n, a, (int)n, b))
#define STR_NEQ(a,b) ASSERT(!__STR_EQ(a,b), "STR_NEQ("#a", "#b")")
#define BUF_NEQ(a,b,n) ASSERT(!__BUF_EQ(a,b,n), "BUF_NEQ("#a", "#b", "#n")")
#define END() do { return; } while(0)

/* Tests global state. */
struct {
	int    all;		    /* Number of all tests */
	int    err;		    /* Number of failed tests */
	int    last_all;	    /* Last test assertions count */
	int    last_err;	    /* Last test assertions fails */
	int    line[TESTMAX];	    /* TEST macros line in file */
	int    todo[TESTMAX];	    /* 1 when test marked as TODO */
	int    verb;		    /* True if verbose mode enabled */
	int    quick;		    /* True if quick mode enabled */
	char  *fname;		    /* Test source file name */
	char  *msg[TESTMAX];	    /* TEST macro messages */
	void (*its[TESTMAX])(void); /* Test functions pointers */
} __test = {0, 0, 0, 0, {0}, {0}, 0, 0, NULL, {NULL}, {NULL}};

/* Runs all tests defined with TEST macro.  Program returns number of
 * failed tests or 0 on success. */
int
main(int argc, char **argv)
{
	int     i;
	char    opt;

	while ((opt = getopt(argc, argv, "vqh")) != -1) {
		switch(opt) {
		case 'v': __test.verb  = 1; break;
		case 'q': __test.quick = 1; break;
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
	for (i = 0; i < __test.all; i++) {
		if (__test.todo[i]) {
			fprintf(stderr, "%s:%d: note:\tTODO %s\n",
				__test.fname, __test.line[i], __test.msg[i]);
			continue;
		}

		__test.last_all = 0;
		__test.last_err = 0;
		(*__test.its[i])();   /* Run, print assertion fails */

		if (__test.last_err) {
			__test.err++;
			fprintf(stderr, "%s:%d: error:\tTEST %s\n",
				__test.fname, __test.line[i], __test.msg[i]);
		}
		if (__test.verb) {
			printf("TEST %s\t(%d/%d) pass\n",
			       __test.msg[i],
			       __test.last_all - __test.last_err,
			       __test.last_all);
		}
	}

	/* Print results summary. */
	if (__test.verb) {
		printf("FILE %s\t(%d/%d) pass\n", __test.fname,
		       __test.all - __test.err, __test.all);
	} else {
		printf("%s\t%d err\n", __test.fname, __test.err);
	}

	return __test.err;
}

#endif	/* WALTER_H_ */
