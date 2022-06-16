/* All tests should fail. */

#include <string.h>
#include "walter.h"

TEST("booleans")
{
	int  bool_t = 1;
	int  bool_f = 0;

	OK(!bool_t);
	OK( bool_f);

	OK(0);
	OK(!1);
	OK(1 != 1);
	OK(0 == 1);

	ASSERT(!bool_t, "Custom fail message");
	ASSERT( bool_f, "Custom fail message");
}

TEST("numbers")
{
	int      num = 123;
	double  fnum = 0.1 + 0.2;

	NEQ(123, 123);
	NEQ(num, 123);
	NEQ(num, num);

	NEQ(1.23, 1.23);
	NEQ(fnum, 0.1 + 0.2);
	NEQ(fnum, fnum);

	EQ(123, 456);
	EQ(num, 456);

	EQ(1.23, -1.23);
	EQ(0.3, 0.1 + 0.2);
	EQ(fnum, num);
}

TEST("strings")
{
	char  *str = "Lorem ipsum";

	STR_NEQ("Lorem ipsum", "Lorem ipsum");
	STR_NEQ(str, "Lorem ipsum");
	STR_NEQ(str, str);
	STR_NEQ(NULL, NULL);

	STR_EQ(str, NULL);
	STR_EQ("Lorem ipsum", NULL);
	STR_EQ("Lorem ipsum", "test");
}

TEST("buffers")
{
	char *str = "The trick is not minding that it hurts.";
	char  buf[16];

	strncpy(buf, str, 16);

	BUF_NEQ("Lorem ipsum", "Lorem ipsum", 10);
	BUF_NEQ(str, str, strlen(str));
	BUF_NEQ(buf, buf, 16);
	BUF_NEQ(buf, buf, 4);
	BUF_NEQ(buf, str, 4);

	BUF_EQ("Lorem ipsum", "Lorem  psum", 10);
	BUF_EQ(str, "Lorem ipsum", 8);
	BUF_EQ(buf, "Lorem ipsum", 8);
}

TEST("flow")
{
	FAIL("Custom fail message");
	END();

	OK(1);			/* Unreachable */
	OK(1);
	OK(1);
}
