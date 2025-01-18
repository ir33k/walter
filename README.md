Walter
======

Single header library for writing unit tests in C with fewer
complications by avoiding boilerplate.

	walter.h        Library with full documentation and licence
	demo/           Demonstration test programs AKA examples
	snap/           Snapshots for walter.t.c library tests
	build           Script to build and run tests

Expected to work on POSIX systems and NOT on Windows.


Example
-------

```c
// File: example.t.c
#include "some_lib.h"           // Include your code
#include "walter.h"             // Include Walter

TEST("Test description")        // Define test with assertsions
{
	OK(func1());            // Fail when value is 0
	EQ(func2(), "abc", 3);  // Fail when buffers of size 3 are not equal
}

// No main() function as it is already defined in walter.h
```

Compile and run:

```sh
$ cc -o example.t example.t.c
$ ./example.t
	 First incorrect byte: 2
	"abb"
	"abc"
example.t.c:8:	EQ("abb", "abc", 3)
example.t.c:5:	TEST Test description
example.t.c	1 err
```

This is examle of default program output when second assertion failed
because string produced by `func2()` was different than `"abc"`. at
third character.  First you get the details about what went wrong,
then path to failed assertion, then path to fialed test with that
assertion.  Lastly there is a summery of how many errors where
produced in that test file.  By default when all tests pass there is
no output.

Full documentation with better example is in `walter.h`.
