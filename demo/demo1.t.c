/** All tests should pass. */

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

	SEQ("Lorem ipsum", "Lorem ipsum");
	SEQ(str, "Lorem ipsum");
	SEQ(str, str);
	SEQ(NULL, NULL);

	SNEQ(str, NULL);
	SNEQ("Lorem ipsum", NULL);
	SNEQ("Lorem ipsum", "test");
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

	END();
	ASSERT(0, "Unreachable fail message");
}
