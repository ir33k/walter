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

	SAME("Lorem ipsum", "Lorem ipsum", -1);
	SAME(str, "Lorem ipsum", -1);
	SAME(str, str, -1);
	SAME(NULL, NULL, -1);

	DIFF(str, NULL, -1);
	DIFF("Lorem ipsum", NULL, -1);
	DIFF("Lorem ipsum", "test", -1);
}

TEST("buffers")
{
	char *str = "The trick is not minding that it hurts.";
	char  buf[16];

	strncpy(buf, str, 16);

	SAME("Lorem ipsum", "Lorem ipsum", 10);
	SAME(str, str, strlen(str));
	SAME(buf, buf, 16);
	SAME(buf, buf, 4);
	SAME(buf, str, 4);

	DIFF("Lorem ipsum", "Lorem  psum", 10);
	DIFF(str, "Lorem ipsum", 8);
	DIFF(buf, "Lorem ipsum", 8);
}

TEST("flow")
{
	OK(1);
	OK(1);
	OK(1);

	return;
	ASSERT(0, "Unreachable fail message");
}
