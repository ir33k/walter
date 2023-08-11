/** Testing standard input, output, error and exit code of command. */

#include "walter.h"

TEST("SRUN on UNIX tools")
{
	SRUN("tr abc 123", "AaBbCc", "A1B2C3", 0, 0);
	SRUN("ls unknown", 0, "", "ls: cannot access 'unknown': No such file or directory\n", 2);
	SRUN("date -u --date='@2147483647'", 0, "Tue 19 Jan 2038 03:14:07 AM UTC\n", 0, 0);
}
