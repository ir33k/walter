/* Simple example of everything you need to know. */

/* Set max number of tests if you need more than default allows. */
#define TESTMAX 1024
#include "walter.h"

TEST("All should pass")		/* Define test with assertions */
{
	int    bool    = 1;
	int    num     = 42;
	char  *str     = "David";
	char   buf[4]  = { '0', '0', '4', '5' };

	OK(bool);		/* Is boolean true? */
	EQ(num, 42);		/* Are numbers equal? */
	STR_EQ(str, "David");	/* Are strings equal? */
	BUF_EQ(buf, "0045", 4);	/* Are buffers equal? */

	DIF(num, 22);	/* Negations */
	STR_DIF(str, "Walter");
	BUF_DIF(buf, "12345", 4);

	/* ASSERT is like OK but with custom fail message. */
	ASSERT(bool, "Fail message");

	END();			/* Force test to end here */
	FAIL("Fail message");	/* Force fail in this line */
}

TEST("You shall not pass!")	/* 🧙 */
{
	OK(0);
	EQ(0.1 + 0.2, 0.3);
	STR_EQ("Lorem ipsum", "Lorem ipusm");
	BUF_EQ("2345", "0045", 4);
	DIF(44, 44);
	STR_DIF("Lorem ipsum", "Lorem ipsum");
	BUF_DIF("1234", "1234", 4);
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

/* There is no "main()" function. */
