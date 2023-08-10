#include "walter.h"

TEST("_wh_malloc_split")
{
	char **res;

	res = _wh_malloc_split("first second third", ' ');
	STR_EQ(res[0], "first");
	STR_EQ(res[1], "second");
	STR_EQ(res[2], "third");
	STR_EQ(res[3], 0);
	free(res);

	res = _wh_malloc_split("some/dir/path/", '/');
	STR_EQ(res[0], "some");
	STR_EQ(res[1], "dir");
	STR_EQ(res[2], "path");
	STR_EQ(res[3], "");
	STR_EQ(res[4], 0);
	free(res);

	res = _wh_malloc_split("string without split char", '_');
	STR_EQ(res[0], "string without split char");
	STR_EQ(res[1], 0);
	free(res);

	res = _wh_malloc_split("", ' ');
	STR_EQ(res[0], "");
	STR_EQ(res[1], 0);
	free(res);
}

TEST("_wh_cmp")
{
	char buf1[4] = { 0, 1, 2, 3 };
	char buf2[6] = { 0, 1, 2, 3, 4, 5 };

	EQ(_wh_cmp(buf1, buf2, 4), -1);
	EQ(_wh_cmp(buf1, buf2, 6), 4);
	buf1[2] = 9;	  /* Make BUF1 not equal in different place */
	EQ(_wh_cmp(buf1, buf2, 4), 2);
}

SKIP("_wh_fdcmp")
{
	/* TODO(irek): To make tests for this function I need to be
	 * able to track output of stdout and stderr. */

	/* It looks like I need another testing function for testing
	 * stdin, stdout and stderr in specific function.  It's
	 * similar to main RUN macro but for functions and without
	 * exit code. */
}

TEST("demo0")
{
	IOE("./demo0.t -h", 0, "snap/d00out", "snap/d00err", 1);
	IOE("./demo0.t",    0, "snap/d01out", "snap/d01err", 3);
	IOE("./demo0.t -v", 0, "snap/d02out", "snap/d02err", 3);
	IOE("./demo0.t -q", 0, "snap/d03out", "snap/d03err", 3);
	IOE("./demo0.t -f", 0, "snap/d04out", "snap/d04err", 1);
}

SKIP("demo1")
{
	/* TODO */
}
