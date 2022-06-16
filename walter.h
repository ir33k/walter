/* Walter is a single header library for writing unit tests in C made
 * with fewer complications.
 *
 * v1.0 from https://github.com/ir33k/walter by Irek (public domain)
 *
 * Example test program:
 *
 *	// File: example.t.c
 *	#define TESTMAX 1024            // Define to handle more tests
 *	#include "walter.h"
 *
 *	TEST("Test message")            // Define test with assertions
 *	{
 *		FAIL("fail message");   // Fail here with message
 *		END();                  // End test here
 *
 *		// TODO(irek): Implement helper functions for creating
 *		// custom assertions.
 *
 *		OK(bool);               // Is boolean true?
 *		EQ(num1, num2);         // Are numbers equal?
 *		STR_EQ(s1, s2);         // Are strings equal?
 *		BUF_EQ(b1, b2, size);   // Are buffers equal?
 *
 *		NOT_EQ(num1, num2);     // Negations
 *		STR_NOT_EQ(s1, s2);
 *		BUF_NOT_EQ(b1, b2, size);
 *
 *		// ASSERT is like OK but with custom fail message
 *		ASSERT(bool, "fail message");
 *
 *		// Helper function that returns boolean
 *		OK(test_str_eq(str1, str2));
 *	}
 *
 *	TEST("Another test 1") { ... }  // Define as many as TESTMAX
 *	TEST("Another test 2") { ... }
 *
 *	// There is no "main" function
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
#include <getopt.h>
#include <string.h>
#include <assert.h>

#ifndef TESTMAX			/* Predefined for more tests */
#define TESTMAX    64		/* Maximum number of tests */
#endif

#define TEST____(_msg, id, _line)                               \
	void test__body##id(void);                              \
	/* This function will be run before "main" function.  It's the
	 * heart, main hack, my own precious ring of power that makes
	 * this test lib possible. */                           \
	void test__##id(void) __attribute__ ((constructor));    \
	void test__##id(void) {                                 \
		if (test__.all == 0) test__init(__FILE__);      \
		test__.msg[test__.all] = _msg;                  \
		test__.line[test__.all] = _line;                \
		test__.its[test__.all] = &test__body##id;       \
		test__.all++;                                   \
	}                                                       \
	void test__body##id(void)
/* Intermediate macro function TEST__ is necessary to "unwrap"
 * __LINE__ macro so it could be used as a ID string. */
#define TEST__(msg, id) TEST____(msg, id, __LINE__)
#define TEST(msg) TEST__(msg, __LINE__)

/* TODO(irek): Create pending test macro.  Call it TODO?  With that
 * number of pending tests should be shown in final summary.  Having
 * more than two number in summary make it convenient to actually show
 * number of passed tests too. */

/* Assertions */
#define ASSERT____(bool, onfail, line) do {                     \
		test__.sum++;					\
		if (bool) break;            /* Pass */		\
		test__.fail++;              /* Fail */		\
		fprintf(stderr, "%s:%d: error: ",		\
			test__.fname, line);			\
		onfail;                                         \
		fputc('\n', stderr);                            \
		/* In -q quick mode stop test on first fail*/	\
		if (test__.quick) return;			\
	} while(0)
#define ASSERT__(bool, onfail) ASSERT____(bool, onfail, __LINE__)

/* Main assertions */
#define ASSERT(x,msg) ASSERT__(x, fputs(msg, stderr))
#define OK(x) ASSERT(x, "'"#x"' is not ok")
/* TODO(irek): Print actuall numbers. */
#define EQ(a,b) ASSERT((a) == (b), "'"#a"' is not equal to '"#b"'")
#define STR_EQ(a,b) ASSERT__(test_str_eq(a, b),			\
			     test__pstrs("strings are not equal", a, b))
#define BUF_EQ(a,b,n) ASSERT__(strncmp(a, b, n) == 0,			\
			       test__pbufs("buffers are not equal", a, b, n))
/* Negations */
#define NOT_EQ(a,b) ASSERT((a) != (b), "'"#a"' number is equal to '"#b"'")
#define STR_NOT_EQ(a,b) ASSERT__(!test_str_eq(a, b),		\
				 test__pstrs("strings are equal", a, b))
#define BUF_NOT_EQ(a,b,n) ASSERT__(strncmp(a, b, n) != 0,		\
				   test__pbufs("buffers are equal", a, b, n))
/* Flow control */
#define FAIL(msg) ASSERT(0, msg)
#define END() do { return; } while(0)

typedef struct {
	int    all;		    /* Number of all tests */
	int    err;		    /* Number of failed tests */
	int    sum;		    /* Last test assertions count */
	int    fail;		    /* Last failed assertions count */
	int    line[TESTMAX];	    /* TEST macros line in file */
	int    verb;		    /* True if verbose mode enabled */
	int    quick;		    /* True if quick mode enabled */
	char  *fname;		    /* Test source file name */
	char  *msg[TESTMAX];	    /* TEST macro messages */
	void (*its[TESTMAX])(void); /* Test functions pointers */
} TestState;

void test__usage(char *argv0);
void test__init(char *fname);
void test__pbufs(char *msg, char *a, char *b, int siz);
void test__pstrs(char *msg, char *a, char *b);
int  test_str_eq(char *a, char *b);

extern TestState test__;         /* Global warning  ^u^  */
TestState test__ = {0, 0, 0, 0, {0}, 0, 0, NULL, {NULL}, {NULL}};

/* Runs all tests defined with TEST macro.  Program returns number of
 * failed tests or 0 on success. */
int
main(int argc, char **argv)
{
	int     i;
	char    opt;

	while ((opt = getopt(argc, argv, "hvq")) != -1) {
		switch(opt) {
		case 'h':
			test__usage(argv[0]);
			return 0;
		case 'v':
			test__.verb = 1;
			break;
		case 'q':
			test__.quick = 1;
			break;
		default:
			test__usage(argv[0]);
			return 1;
		}
	}

	/* TODO(irek): This should probably be done inside TEST macro.
	 * I'm not sure who program will react to exit(1) outside of
	 * "main" function. */
	if (test__.all > TESTMAX) {
		fprintf(stderr, "ERROR: More than %d tests (read doc)\n",
			TESTMAX);
		return 1;
	}

	/* Run tests.  Print error on fail. */
	for (i = 0; i < test__.all; i++) {
		test__.sum = 0;
		test__.fail = 0;
		(*test__.its[i])();   /* Run, print assertion fails */

		if (test__.fail) {
			test__.err++;
			fprintf(stderr, "%s:%d: error: TEST %s\n",
				test__.fname, test__.line[i], test__.msg[i]);
		}
		if (test__.verb) {
			printf("TEST %s\t(%d/%d) pass\n",
			       test__.msg[i],
			       test__.sum - test__.fail,
			       test__.sum);
		}
	}

	/* Print results summary */
	if (test__.verb) {
		printf("FILE %s\t(%d/%d) pass\n",
		       test__.fname,
		       test__.all - test__.err,
		       test__.all);
	} else {
		printf("%s\t%d err\n", test__.fname, test__.err);
	}

	return test__.err;
}

/* Print usage help message with given ARGV0 program name. */
void
test__usage(char *argv0)
{
	printf("usage: %s [-hvq]\n\n"
	       "\t-h\tPrints this help usage message.\n"
	       "\t-v\tRun in verbose mode.\n"
	       "\t-q\tStop current test on first fail.\n",
	       argv0);
}

/* Initialize test global state on first TEST macro.  Define FNAME
 * test file name with value of __FILE__ macro so we get source file
 * name instead of program file name like we have with argv[0]. */
void
test__init(char *fname)
{
	test__.fname = fname;
}

/* Print to stderr MSG message with buffers A and B of given SIZ size
 * in separate lines to make differences easily visible. */
void
test__pbufs(char *msg, char *a, char *b, int siz)
{
	fprintf(stderr, "%s:\n\t'%-*s'\n\t'%-*s'",
		msg, siz, a, siz, b);
}

/* Print to stderr MSG message with strings A and B in separate lines
 * to make differences easily visible. */
void
test__pstrs(char *msg, char *a, char *b)
{
	fprintf(stderr, "%s:\n\t'%s'\n\t'%s'",
		msg, a ? a : "<NULL>", b ? b : "<NULL>");
}

/* Check if given strings A and B are equal or both are NULL. */
int
test_str_eq(char *a, char *b)
{
	return (a && b && strcmp(a, b) == 0) || (!a && !b);
}

#endif	/* WALTER_H_ */
