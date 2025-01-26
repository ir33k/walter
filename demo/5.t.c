/* Testing standard input, output, error and exit code of command. */

#include "../walter.h"

TEST("RUN with string literals on UNIX tools")
{
	RUN("tr abc 123", STR"AaBbCc", STR"A1B2C3", 0, 0);
	RUN("ls unknown", 0, STR"", STR"ls: cannot access 'unknown': No such file or directory\n", 2);
	RUN("LC_TIME=en_US.UTF-8 date -u --date='@2147483647'", 0, STR"Tue Jan 19 03:14:07 AM UTC 2038\n", 0, 0);
}

TEST("Fail to demonstrate error messages")
{
	RUN("tr abc 123", STR"AaBbCc", STR"A1B_C3", 0, 0);
	RUN("ls unknown", 0, 0, 0, 2);
	RUN("ls /", 0, 0, 0, 1);
}
