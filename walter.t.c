#include "walter.h"

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
