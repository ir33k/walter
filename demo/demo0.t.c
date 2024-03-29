/** Simple examples of basic assertions. */

/* Set max number of tests by defining TESTMAX before including
 * walter.h if you need more than 64. */
#define TESTMAX 128
#include "../walter.h"

TEST("All should pass")		/* Define test with assertions */
{
	int    bool    = 1;
	char  *str     = "David";
	char   buf[4]  = { '0', '0', '4', '5' };

	OK(bool);		/* Is boolean true? */
	ASSERT(bool, "text");	/* OK() with custom message */
	SEQ(str, "David");	/* Are strings equal? */
	EQ(buf, "0045", 4);	/* Are buffers equal? */

	SNEQ(str, "Walter");	/* Are strings not equal? */
	NEQ(buf, "123", 3);	/* Are buffers not equal? */

	END();			/* Force test to end here */
	ASSERT(0, "Fail");	/* Force fail in this line */
}

TEST("You shall not pass!")	/* All should fail */
{
	OK(0);
	OK(0.1 + 0.2 == 0.3);
	OK(44 != 44);
	SEQ("Lorem ipsum", "Lorem ipusm");
	SEQ("Lorem ipsumm", "Lorem ipsum");
	EQ("2345", "0045", 4);
	SNEQ("Lorem ipsum", "Lorem ipsum");
	NEQ("1234", "1234", 4);
	ASSERT(0, "Custom fail message");
}

TEST("Trigger fail at any moment")
{
	OK(1);
	OK(1);
	OK(1);

	if (1) {
		ASSERT(0, "Fail"); /* Fail and continue */
	}

	ASSERT(0, "Second fail");
	ASSERT(0, "Third fail");
}

TEST("End test at any moment")
{
	OK(1);
	OK(1);
	OK(1);

	if (1) {
		END();		/* End here */
	}

	OK(0);			/* Unreachable */
	OK(0);
}

TEST("Fail and end at the same time")
{
	OK(1);
	OK(1);
	OK(1);

	if (1) {
		ASSERT(0, "Fail");
		END();
	}

	OK(0);			/* Unreachable */
	OK(0);
}

SKIP("Skip or mark any test as TODO") {}

SKIP("Not finished or just ignored test") {
	ASSERT(0, "Fail message"); /* Unreachable */
	ASSERT(0, "Fail message");
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
 *	$ ./demo0.t -q              # End quick on first failed assertion
 *	$ ./demo0.t -vq             # Captain obvious
 *	$ ./demo0.t -f              # End entire testing on first failed test
 */
