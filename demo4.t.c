/** Only ONLY() tests should run. */

#include "walter.h"

TEST("This test will be ignored")
{
	OK(0);			/* Ignored fail */
}

SKIP("This test will be ignored")
{
	OK(0);			/* Ignored skip */
}

ONLY("This test will succeed")
{
	OK(1);			/* Succeed */
}

ONLY("This test will fail")
{
	OK(0);			/* Fail */
}

TEST("This test will be ignored")
{
	OK(0);			/* Ignored fail */
}
