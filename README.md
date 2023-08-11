walter
======

Single header library for writing unit tests in C made with fewer
complications by avoiding boilerplate.  Comparing to other similar
libraries this focus on minimizing tests setup and prints direct paths
with line number to failed assertions and tests.

I would like to say that code is simple but it's mostly macro magic
and global state.  So better hold your wizards hat while reading.

Detailed documentation can be found inside library file `walter.h`.
Examples can be found in `demo/` and `demo.t.c`.

	walter.h        Library, includes licence and documentation
	build           Script to builds demo test programs and demo.t.c
	demo/           Demonstration programs
	snap/           Snapshots of expected output in demo.t.c tests
	
Should work on POSIX systems.  
Should NOT work on Windows.
