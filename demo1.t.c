/* All tests should pass. */

#include <string.h>
#include "walter.h"

TEST("booleans")
{
	int  bool_t = 1;
	int  bool_f = 0;

	OK( bool_t);
	OK(!bool_f);

	OK(1);
	OK(!0);
	OK(1 == 1);
	OK(0 != 1);

	ASSERT( bool_t, "Custom fail message");
	ASSERT(!bool_f, "Custom fail message");
}

TEST("numbers")
{
	int      num = 123;
	double  fnum = 0.1 + 0.2;

	EQ(123, 123);
	EQ(num, 123);
	EQ(num, num);

	EQ(1.23, 1.23);
	EQ(fnum, 0.1 + 0.2);
	EQ(fnum, fnum);

	NOT_EQ(123, 456);
	NOT_EQ(num, 456);

	NOT_EQ(1.23, -1.23);
	NOT_EQ(0.3, 0.1 + 0.2);
	NOT_EQ(fnum, num);
}

TEST("strings")
{
	char  *str = "Lorem ipsum";

	STR_EQ("Lorem ipsum", "Lorem ipsum");
	STR_EQ(str, "Lorem ipsum");
	STR_EQ(str, str);
	STR_EQ(NULL, NULL);

	STR_NOT_EQ(str, NULL);
	STR_NOT_EQ("Lorem ipsum", NULL);
	STR_NOT_EQ("Lorem ipsum", "test");
}

TEST("buffers")
{
	char *str = "The trick is not minding that it hurts.";
	char  buf[16];

	strncpy(buf, str, 16);

	BUF_EQ("Lorem ipsum", "Lorem ipsum", 10);
	BUF_EQ(str, str, strlen(str));
	BUF_EQ(buf, buf, 16);
	BUF_EQ(buf, buf, 4);
	BUF_EQ(buf, str, 4);

	BUF_NOT_EQ("Lorem ipsum", "Lorem  psum", 10);
	BUF_NOT_EQ(str, "Lorem ipsum", 8);
	BUF_NOT_EQ(buf, "Lorem ipsum", 8);
}

TEST("flow")
{
	OK(1);
	OK(1);
	OK(1);

	END();
	FAIL("Unreachable fail message");
}
