/* Who guards the guard?

Tests for Walter test lib.  It validates output of demonstration test
files using walter.h as they do.  What can go wrong (^_^ )
*/
#include "walter.h"

#define SRC "./demo/"	/* Path to demonstratino files */

TEST("demo0")
{
	RUN(SRC"demo0.t -h", 0, "snap/empty",      "snap/demo0.0err", 1);
	RUN(SRC"demo0.t",    0, "snap/empty",      "snap/demo0.1err", 3);
	RUN(SRC"demo0.t -v", 0, "snap/demo0.2out", "snap/demo0.2err", 3);
	RUN(SRC"demo0.t -q", 0, "snap/empty",      "snap/demo0.3err", 3);
	RUN(SRC"demo0.t -f", 0, "snap/empty",      "snap/demo0.4err", 1);
	/* I can't check absolute path feature as it will produce
	 * different results on different machines.  Shame (-_- )*/
	/* RUN(SRC"demo0.t -a", 0, 0, 0, 0); */
}

TEST("demo1")
{
	RUN(SRC"demo1.t",    0, "snap/empty",      "snap/empty", 0);
	RUN(SRC"demo1.t -v", 0, "snap/demo1.0out", "snap/empty", 0);
}

TEST("demo2")
{
	RUN(SRC"demo2.t",     0, "snap/empty",      "snap/demo2.0err", 5);
	RUN(SRC"demo2.t -qv", 0, "snap/demo2.1out", "snap/demo2.1err", 5);
}

TEST("demo3")
{
	RUN(SRC"demo3.t -v", 0, "snap/demo3.0out", "snap/demo3.0err", 3);
}

TEST("demo4")
{
	RUN(SRC"demo4.t -v", 0, "snap/demo4.0out", "snap/demo4.0err", 1);
}

TEST("demo5")
{
	RUN(SRC"demo5.t", 0, "snap/empty", "snap/empty", 0);
}
