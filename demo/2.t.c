/* All tests should fail. */

#include <string.h>
#include "../walter.h"

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

	OK(123 != 123);
	OK(num != 123);
	OK(num != num);
	OK(num <= 100);

	OK(1.23 != 1.23);
	OK(fnum != 0.1 + 0.2);
	OK(fnum != fnum);

	OK(123 == 456);
	OK(num == 456);

	OK(1.23 == -1.23);
	OK(0.3  == 0.1 + 0.2);
	OK(fnum == num);
}

TEST("strings")
{
	char *str = "Lorem ipsum";

	DIFF("Lorem ipsum", "Lorem ipsum", -1);
	DIFF(str, "Lorem ipsum", -1);
	DIFF(str, str, -1);
	DIFF(NULL, NULL, -1);

	SAME(str, NULL, -1);
	SAME("Lorem ipsum", NULL, -1);
	SAME("Lorem ipsum", "test", -1);
	SAME("Lorem ipsum", "Lorem ipsumm", -1);
	SAME("Lorem ipsum", "lorem ipsum", -1);

	SAME("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Ut sodales consequat nulla et sollicitudin. Cras sit amet ligula sapien. In quis ultrices purus. Morbi sodales at velit vulputate aliquam.",
	   "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Ut sodales consequat nulla et sollicitudin. Cras sit amet ligula Sapien. In quis ultrices purus. Morbi sodales at velit vulputate aliquam.", -1);
}

TEST("buffers")
{
	char *str = "The trick is not minding that it hurts.";
	char  buf[16];

	strncpy(buf, str, 16);

	DIFF("Lorem ipsum", "Lorem ipsum", 10);
	DIFF(str, str, strlen(str));
	DIFF(buf, buf, 16);
	DIFF(buf, buf, 4);
	DIFF(buf, str, 4);

	SAME("Lorem ipsum", "Lorem  psum", 10);
	SAME(str, "Lorem ipsum", 8);
	SAME(buf, "Lorem ipsum", 8);
}

TEST("flow")
{
	ASSERT(0, "Custom fail message");
	return;

	OK(1);			/* Unreachable */
	OK(1);
	OK(1);
}
