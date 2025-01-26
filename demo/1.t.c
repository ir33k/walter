/* All tests should pass. */

#include <string.h>
#include "../walter.h"

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

	OK(123 == 123);
	OK(num == 123);
	OK(num == num);
	OK(num >= 100);

	OK(1.23 == 1.23);
	OK(fnum == 0.1 + 0.2);
	OK(fnum == fnum);

	OK(123 != 456);
	OK(num != 456);

	OK(1.23 != -1.23);
	OK(0.3  != 0.1 + 0.2);
	OK(fnum != num);
}

TEST("strings")
{
	char  *str = "Lorem ipsum";

	EQ("Lorem ipsum", "Lorem ipsum", -1);
	EQ(str, "Lorem ipsum", -1);
	EQ(str, str, -1);
	EQ(NULL, NULL, -1);

	NEQ(str, NULL, -1);
	NEQ("Lorem ipsum", NULL, -1);
	NEQ("Lorem ipsum", "test", -1);
}

TEST("buffers")
{
	char *str = "The trick is not minding that it hurts.";
	char  buf[16];

	strncpy(buf, str, 16);

	EQ("Lorem ipsum", "Lorem ipsum", 10);
	EQ(str, str, strlen(str));
	EQ(buf, buf, 16);
	EQ(buf, buf, 4);
	EQ(buf, str, 4);

	NEQ("Lorem ipsum", "Lorem  psum", 10);
	NEQ(str, "Lorem ipsum", 8);
	NEQ(buf, "Lorem ipsum", 8);
}

TEST("flow")
{
	OK(1);
	OK(1);
	OK(1);

	return;
	ASSERT(0, "Unreachable fail message");
}
