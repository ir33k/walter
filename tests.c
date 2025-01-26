/* Who guards the guard?

Tests for Walter test lib.  It validates output of demonstration test
files using walter.h as they do.  What can go wrong (^_^ )
*/
#include "walter.h"

TEST("Example demonstration tests should produce expected output")
{
	RUN("demo/0.t -h",   0, "snap/0a",    0, 1);
	RUN("demo/0.t",      0, "snap/0b",    0, 3);
	RUN("demo/0.t -q",   0, "snap/0c",    0, 3);
	RUN("demo/0.t -l 1", 0, "snap/0d",    0, 1);
	RUN("demo/1.t",      0, "snap/empty", 0, 0);
	RUN("demo/2.t",      0, "snap/2a",    0, 5);
	RUN("demo/2.t -q",   0, "snap/2b",    0, 5);
	RUN("demo/3.t",      0, "snap/3a",    0, 3);
	RUN("demo/4.t",      0, "snap/4a",    0, 1);
	RUN("demo/5.t",      0, "snap/empty", 0, 0);
}
