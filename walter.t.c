#include "walter.h"
#include "david.h"

TEST("demo0")
{
	IOE("./demo0.t -h", NULL, "snap/d00out", "snap/d00err", 1);
	IOE("./demo0.t",    NULL, "snap/d01out", "snap/d01err", 3);
	IOE("./demo0.t -v", NULL, "snap/d02out", "snap/d02err", 3);
	IOE("./demo0.t -q", NULL, "snap/d03out", "snap/d03err", 3);
	IOE("./demo0.t -f", NULL, "snap/d04out", "snap/d04err", 1);
}

SKIP("demo1")
{
	/* TODO */
}
