/* Simple example of everything you need to know. */

/* Set max number of tests if you need more than 64. */
#define TESTMAX 128
#include "walter.h"

TEST("All should pass")		/* Define test with assertions */
{
	int    bool    = 1;
	int    num     = 42;
	char  *str     = "David";
	char   buf[4]  = { '0', '0', '4', '5' };

	OK(bool);		/* Is boolean true? */
	ASSERT(bool, "text");	/* OK() with custom message */
	EQ(num, 42);		/* Are numbers equal? */
	STR_EQ(str, "David");	/* Are strings equal? */
	BUF_EQ(buf, "0045", 4);	/* Are buffers equal? */

	NEQ(num, 22);		/* Are numbers not equal? */
	STR_NEQ(str, "Walter");	/* Are strings not equal? */
	BUF_NEQ(buf, "123", 3);	/* Are buffers not equal? */

	END();			/* Force test to end here */
	FAIL("Fail message");	/* Force fail in this line */
}

TEST("You shall not pass!")	/* All should fail */
{
	OK(0);
	EQ(0.1 + 0.2, 0.3);
	STR_EQ("Lorem ipsum", "Lorem ipusm");
	BUF_EQ("2345", "0045", 4);
	NEQ(44, 44);
	STR_NEQ("Lorem ipsum", "Lorem ipsum");
	BUF_NEQ("1234", "1234", 4);
	ASSERT(0, "Custom fail message");
}

TEST("Trigger fail at any moment")
{
	OK(1);
	OK(1);
	OK(1);

	if (1) {
		FAIL("Fail");	/* Fail, but continue */
	}

	FAIL("Second fail");
	FAIL("Third fail");
}

TEST("End test at any moment")
{
	OK(1);
	OK(1);
	OK(1);

	if (1) {
		END();		/* End here */
	}

	FAIL("Fail message");	/* Unreachable */
	FAIL("Fail message");
}

TEST("Fail and end at the same time")
{
	OK(1);
	OK(1);
	OK(1);

	if (1) {
		FAIL("Fail");
		END();
	}

	FAIL("Fail message");	/* Unreachable */
	FAIL("Fail message");
}

TODO("Mark any test as TODO") {}

TODO("Not finished or just ignored test") {
	FAIL("Fail message");	/* Unreachable */
	FAIL("Fail message");
}

/*
 * There is no "main()" function.
 */

/* Compile and run:
 *
 *	$ cc -o demo0.t demo0.t.c   # Compile
 *	$ ./demo0.t -h              # Print usage help
 *	$ ./demo0.t                 # Run tests
 *	$ ./demo0.t -v              # Run in verbose mode
 *	$ ./demo0.t -q              # End quick on first fail
 *	$ ./demo0.t -vq             # Captain obvious
 */
