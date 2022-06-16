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

	DIF(123, 123);
	DIF(num, 123);
	DIF(num, num);

	DIF(1.23, 1.23);
	DIF(fnum, 0.1 + 0.2);
	DIF(fnum, fnum);

	EQ(123, 456);
	EQ(num, 456);

	EQ(1.23, -1.23);
	EQ(0.3, 0.1 + 0.2);
	EQ(fnum, num);
}

TEST("strings")
{
	char  *str = "Lorem ipsum";

	STR_DIF("Lorem ipsum", "Lorem ipsum");
	STR_DIF(str, "Lorem ipsum");
	STR_DIF(str, str);
	STR_DIF(NULL, NULL);

	STR_EQ(str, NULL);
	STR_EQ("Lorem ipsum", NULL);
	STR_EQ("Lorem ipsum", "test");
}

TEST("buffers")
{
	char *str = "The trick is not minding that it hurts.";
	char  buf[16];

	strncpy(buf, str, 16);

	BUF_DIF("Lorem ipsum", "Lorem ipsum", 10);
	BUF_DIF(str, str, strlen(str));
	BUF_DIF(buf, buf, 16);
	BUF_DIF(buf, buf, 4);
	BUF_DIF(buf, str, 4);

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
