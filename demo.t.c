#include "walter.h"

TEST("All should pass")
{
	OK(1);
	OK(1);
	OK(1);
	OK(1);
}

TEST("Test strings")
{
	STR_EQ("Lorem ipsum", "Lorem ipsum");
	STR_EQ(NULL, NULL);
	/* STR_NOT_EQ("Lorem ipsum", NULL); */
}

TEST("Multiple fails")
{
	OK(1);
	FAIL("first fail");
	OK(1);
	FAIL("second fail");
	FAIL("third fail");
	OK(1);
}

TEST("Should fail")
{
	OK(1);
	FAIL("force fail at this point");
	OK(1);
	OK(1);
}

TEST("Make it pass no matter what")
{
	OK(1);
	PASS();			/* Pass at this point, ignore rest */
	FAIL("ignored");
	FAIL("ignored");
}
