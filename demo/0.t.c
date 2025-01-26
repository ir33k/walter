/* Simple examples of basic assertions. */
#include "../walter.h"

TEST("All should pass")		/* Define test with assertions */
{
	int    bool    = 1;
	char  *str     = "David";
	char   buf[4]  = { '0', '0', '4', '5' };

	OK(bool);		/* Is boolean true? */
	ASSERT(bool, "text");	/* OK() with custom message */
	SAME(str, "David", -1);	/* Are strings equal? */
	SAME(buf, "0045", 4);	/* Are buffers equal? */

	DIFF(str, "Walter", -1);	/* Are strings not equal? */
	DIFF(buf, "123", 3);	/* Are buffers not equal? */

	return;			/* Force test to end here */
	ASSERT(0, "Fail");	/* Force fail in this line */
}

TEST("You shall not pass!")	/* All should fail */
{
	OK(0);
	OK(0.1 + 0.2 == 0.3);
	OK(44 != 44);
	SAME("Lorem ipsum", "Lorem ipusm", -1);
	SAME("Lorem ipsumm", "Lorem ipsum", -1);
	SAME("2345", "0045", 4);
	DIFF("Lorem ipsum", "Lorem ipsum", -1);
	DIFF("1234", "1234", 4);
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
		return;		/* End here */
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
		return;
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
 *	$ ./demo0.t -q              # End quick on first failed assertion
 *	$ ./demo0.t -f              # End entire testing on first failed test
 */
