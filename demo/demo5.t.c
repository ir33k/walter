/** Testing standard input, output, error and exit code of command. */

#include "../walter.h"

TEST("RUN with string literals on UNIX tools")
{
	RUN("tr abc 123", S2F("AaBbCc"), S2F("A1B2C3"), 0, 0);
	RUN("ls unknown", 0, S2F(""), S2F("ls: cannot access 'unknown': No such file or directory\n"), 2);
	RUN("LC_TIME=en_US.UTF-8 date -u --date='@2147483647'", 0, S2F("Tue Jan 19 03:14:07 AM UTC 2038\n"), 0, 0);
}
