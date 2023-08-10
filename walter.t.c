#include "walter.h"

TEST("_wh_cmp")
{
	char buf1[4] = { 0, 1, 2, 3 };
	char buf2[6] = { 0, 1, 2, 3, 4, 5 };

	EQ(_wh_cmp(buf1, buf2, 4), -1);	/* Equal up to size of 4 */
	EQ(_wh_cmp(buf1, buf2, 6), 4);	/* Not equal for size of 6 */
	buf1[2] = 9;	/* Make BUF1 not equal in different place */
	EQ(_wh_cmp(buf1, buf2, 4), 2);	/* Not equal at index of 2 */
}

TEST("STR_RUN on UNIX tools")
{
	STR_RUN("tr abc 123", "AaBbCc", "A1B2C3", 0, 0);
	STR_RUN("ls unknown", 0, "", "ls: cannot access 'unknown': No such file or directory\n", 2);
	STR_RUN("date -u --date='@2147483647'", 0, "Tue 19 Jan 2038 03:14:07 AM UTC\n", 0, 0);
}

TEST("demo0")
{
	RUN("./demo0.t -h", 0, "snap/empty",      "snap/demo0.0err", 1);
	RUN("./demo0.t",    0, "snap/empty",      "snap/demo0.1err", 3);
	RUN("./demo0.t -v", 0, "snap/demo0.2out", "snap/demo0.2err", 3);
	RUN("./demo0.t -q", 0, "snap/empty",      "snap/demo0.3err", 3);
	RUN("./demo0.t -f", 0, "snap/empty",      "snap/demo0.4err", 1);
}

TEST("demo1")
{
	RUN("./demo1.t",    0, "snap/empty",      "snap/empty", 0);
	RUN("./demo1.t -v", 0, "snap/demo1.0out", "snap/empty", 0);
}

TEST("demo2")
{
	RUN("./demo2.t",     0, "snap/empty",      "snap/demo2.0err", 5);
	RUN("./demo2.t -qv", 0, "snap/demo2.1out", "snap/demo2.1err", 5);
}

TEST("demo3")
{
	RUN("./demo3.t -v", 0, "snap/demo3.0out", "snap/demo3.0err", 3);
}

TEST("demo4")
{
	RUN("./demo4.t -v", 0, "snap/demo4.0out", "snap/demo4.0err", 1);
}
